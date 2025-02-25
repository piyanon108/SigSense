#include "OutputTrigger.h"

OutputTrigger::OutputTrigger(QObject *parent)
    : QObject(parent)
{

}


int OutputTrigger::setRelayTrigger()
{
    unsigned int line_num = 0;
    struct gpiod_chip *chip;
    struct gpiod_line *line;
    int ret;

    chip = gpiod_chip_open_by_name("gpiochip2");
    if (!chip) {
     qDebug() << ("Open chip failed\n");
     goto end;
    }

    line = gpiod_chip_get_line(chip, line_num);
    if (!line) {
     qDebug() << ("Get line failed\n");
     goto close_chip;
    }
    ret = gpiod_line_request_output(line, CONSUMER,1);
    if (ret < 0) {
     qDebug() << ("Request line as output failed\n");
     goto release_line;
    }

    ret = gpiod_line_set_value(line,0);
//    qDebug() << "gpiod_line_reset_value" << ret;


    ret = gpiod_line_set_value(line,1);
//    qDebug() << "gpiod_line_set_value" << ret;
    gpiod_chip_close(chip);

    return 0;

release_line:
    gpiod_line_release(line);
close_chip:
    gpiod_chip_close(chip);
end:
 return 0;

}

void OutputTrigger::keymasterPress(QString message)
{
    setRelayTrigger();
    QString formattedDateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz");
    qDebug() << "keypress" <<  formattedDateTime << message;

}
