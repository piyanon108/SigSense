#include "GPIOKeyEvent.h"

void GPIOKeyEvent::run() {
    char *chipname = INPUTGPIOCHIP;
    unsigned int line_num = INPUTGPIOLINE;
    struct timespec ts = { 1, 0 };
    struct gpiod_line_event event;
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;
    int fd;
    uint32_t read_result = -1;
    void *map_base, *virt_addr;
    unsigned offset;
    unsigned int pagesize = (unsigned)sysconf(_SC_PAGESIZE);
    unsigned int map_size = pagesize;
    uint64_t target = 0x41210000;

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    offset = (unsigned int)(target & (pagesize - 1));

    map_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
            fd,
            target & ~((typeof(target))pagesize - 1));

    virt_addr = map_base + offset;


    chip = gpiod_chip_open_by_name(chipname);
    if (!chip) {
        perror("Open chip failed\n");
        ret = -1;
        goto end;
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
        perror("Get line failed\n");
        ret = -1;
        goto close_chip;
    }

    ret = gpiod_line_request_falling_edge_events(line, CONSUMER);
    if (ret < 0) {
        perror("gpiod_line_request_falling_edge_events Request event notification failed\n");
        ret = -1;
        goto release_line;
    }

    /* Notify event up to 20 times */
    while (1)
    {
        ret = gpiod_line_event_wait(line, &ts);
        if (ret < 0) {
            //perror("Wait event notification failed\n");
            ret = -1;
            goto release_line;
        } else if (ret == 0) {
            //printf("Wait event notification on line #%u timeout\n", line_num);
            continue;
        }

        ret = gpiod_line_event_read(line, &event);

        if (ret < 0)
        {
            //perror("Read last event notification failed\n");
            ret = -1;
            goto release_line;
        }
        else
        {
            read_result = *((volatile uint32_t *)virt_addr);
            emit keypress(read_result);
        }
    }

    ret = 0;

release_line:
    gpiod_line_release(line);
close_chip:
    gpiod_chip_close(chip);
end:
    return;
}
