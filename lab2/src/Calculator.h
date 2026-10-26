#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QString>
#include <QWidget>

class QGridLayout;
class QPlainTextEdit;
class QPushButton;
class QRadioButton;

// окно калькулятора с двумя режимами: обычным и инженерным
class Calculator : public QWidget
{
    Q_OBJECT

public:
    // конструктор: создаёт дисплей, переключатель режимов, все кнопки
    // и включает обычный режим
    explicit Calculator(QWidget* p_parent = nullptr);

private:
    // создаёт кнопку с надписью; p_kind - вид кнопки для стилей qss
    QPushButton* createButton(const QString& p_text, const QString& p_kind);

    // создаёт кнопку цифры и ставит её в сетку
    QPushButton* addDigitButton(QGridLayout* p_grid, const QString& p_digit,
                                int p_row, int p_column, int p_columnSpan = 1);

    // создаёт кнопку операции над одним числом (sqrt, sin, ln...) и ставит её в сетку
    QPushButton* addUnaryButton(QGridLayout* p_grid, const QString& p_text,
                                const QString& p_operation, int p_row, int p_column);

    // создаёт кнопку операции над двумя числами (+, -, xʸ...) и ставит её в сетку
    QPushButton* addBinaryButton(QGridLayout* p_grid, const QString& p_text,
                                 const QString& p_operation, int p_row, int p_column,
                                 const QString& p_kind = "operation");

    // создаёт сетку кнопок обычного режима: цифры, + - * /, sqrt, 1/x, =
    QGridLayout* createNumberPanel();

    // создаёт панель кнопок инженерного режима: sin, cos, ln, n! и т.д.
    QWidget* createEngineeringPanel();

    // переключает режим: показывает или прячет инженерную панель и меняет заголовок
    void setEngineeringMode(bool p_engineering);

    // нажата цифра
    void digitClicked(const QString& p_digit);

    // нажата точка
    void pointClicked();

    // нажата операция над двумя числами: + - * / xʸ ʸ√x
    void binaryOperationClicked(const QString& p_operation);

    // нажата операция над одним числом: sqrt, 1/x, sin, ln, n! и т.д.
    void unaryOperationClicked(const QString& p_operation);

    // нажата кнопка "="
    void equalClicked();

    // нажата кнопка π
    void piClicked();

    // нажата кнопка C: сбрасывает всё
    void clearAll();

    // выполняет отложенную операцию: m_result = m_result (операция) p_rightOperand,
    // возвращает false, если результат посчитать нельзя (например, деление на ноль)
    bool calculate(double p_rightOperand);

    // показывает число на дисплее (или ошибку, если это не число)
    void showNumber(double p_value);

    // показывает ошибку и сбрасывает калькулятор
    void showError();

    // число, которое сейчас на дисплее
    double displayValue() const;

    QPlainTextEdit* m_display;        // дисплей
    QRadioButton* m_normalRadio;      // переключатель "Обычный"
    QRadioButton* m_engineeringRadio; // переключатель "Инженерный"
    QWidget* m_engineeringPanel;      // инженерные кнопки, видны только в инженерном режиме
    QPushButton* m_sqrtButton;        // кнопка корня: её надпись зависит от режима

    double m_result;              // левое число операции, которая ждёт второе число
    QString m_pendingOperation;   // эта операция ("+", "x^y"...), пусто - если её нет
    bool m_waitingForOperand;     // true - следующая цифра начинает новое число
    bool m_newNumberEntered;      // true - после последней операции появилось новое число
};

#endif // CALCULATOR_H
