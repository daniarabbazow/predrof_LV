#include "Calculator.h"

#include <QApplication>

// главная функция: создаёт приложение Qt и показывает окно калькулятора
int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    Calculator calculator;
    calculator.show();

    // цикл обработки событий: нажатия кнопок, перерисовка окна и т.д.
    return app.exec();
}
