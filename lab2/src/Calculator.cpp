#include "Calculator.h"

#include <QFont>
#include <QFontMetrics>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSizePolicy>
#include <QTextDocument>
#include <QTextOption>
#include <QVBoxLayout>

#include <cmath>
#include <limits>

// число π
const double PI = 3.14159265358979323846;

// сколько символов можно набрать на дисплее
const int MAX_LENGTH = 15;

// стили qss: оформление кнопок, дисплея и рамки переключателя.
// свойство kind у кнопки (digit, operation, function...) задаётся в createButton
const char* const STYLE_SHEET = R"(
QPlainTextEdit {
    background: white;
    border: 1px solid #9a9a9a;
    border-radius: 3px;
    padding: 2px 6px;
    color: #1a1a1a;
}

QGroupBox {
    border: 1px solid #c4c4c4;
    border-radius: 3px;
    background: #fafafa;
}

QPushButton {
    min-width: 50px;
    min-height: 38px;
    font-size: 15px;
    border: 1px solid #a8a8a8;
    border-radius: 4px;
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #ffffff, stop:1 #dddddd);
}

QPushButton:hover {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #f2f8ff, stop:1 #cfe2f6);
}

QPushButton:pressed {
    background: #b9d2ec;
}

QPushButton[kind="operation"], QPushButton[kind="equal"] {
    font-weight: bold;
}

QPushButton[kind="function"] {
    color: #1c3f73;
}

QPushButton[kind="equal"] {
    background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #eaf2fc, stop:1 #b7d0ec);
}

QPushButton[kind="clear"] {
    color: #a31515;
    font-weight: bold;
}
)";

// факториал целого числа от 0 до 170 (дальше не помещается в double),
// для остальных чисел возвращает NaN, а он потом покажется как ошибка
double factorial(double p_n)
{
    if (p_n < 0 || p_n > 170 || p_n != std::floor(p_n))
        return std::numeric_limits<double>::quiet_NaN();

    double result = 1.0;
    for (int i = 2; i <= static_cast<int>(p_n); ++i)
        result *= i;

    return result;
}

// --------------------------------------------------------- создание окна

// сверху вниз: дисплей, строка с переключателем режимов и кнопкой C,
// строка с инженерной панелью и сеткой обычных кнопок
Calculator::Calculator(QWidget* p_parent)
    : QWidget(p_parent)
    , m_result(0.0)
    , m_waitingForOperand(true)
    , m_newNumberEntered(false)
{
    // дисплей: только для чтения, моноширинный шрифт, текст прижат вправо
    m_display = new QPlainTextEdit("0");
    m_display->setReadOnly(true);
    m_display->setFocusPolicy(Qt::NoFocus);
    m_display->setLineWrapMode(QPlainTextEdit::NoWrap);
    m_display->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_display->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QFont displayFont("Consolas", 22);
    displayFont.setStyleHint(QFont::Monospace);   // если Consolas нет - любой моноширинный
    m_display->setFont(displayFont);
    m_display->document()->setDefaultTextOption(QTextOption(Qt::AlignRight));
    m_display->setFixedHeight(QFontMetrics(displayFont).height() + 22);

    // переключатель режимов: две радиокнопки в рамке, по умолчанию - обычный
    m_normalRadio = new QRadioButton("Обычный");
    m_engineeringRadio = new QRadioButton("Инженерный");
    m_normalRadio->setChecked(true);

    QGroupBox* modeBox = new QGroupBox;
    QVBoxLayout* modeLayout = new QVBoxLayout(modeBox);
    modeLayout->addWidget(m_normalRadio);
    modeLayout->addWidget(m_engineeringRadio);

    QPushButton* clearButton = createButton("C", "clear");
    connect(clearButton, &QPushButton::clicked, this, &Calculator::clearAll);

    QHBoxLayout* modeRow = new QHBoxLayout;
    modeRow->addWidget(modeBox, 1);   // 1 - рамка забирает всё свободное место
    modeRow->addWidget(clearButton);

    // кнопки: слева инженерная панель, справа обычные кнопки
    m_engineeringPanel = createEngineeringPanel();

    QHBoxLayout* buttonsRow = new QHBoxLayout;
    buttonsRow->addWidget(m_engineeringPanel);
    buttonsRow->addLayout(createNumberPanel());

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_display);
    mainLayout->addLayout(modeRow);
    mainLayout->addLayout(buttonsRow);

    // размер окна всегда равен размеру содержимого: когда инженерная панель
    // прячется или появляется, окно само становится уже или шире
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    // toggled(bool) передаёт true, когда выбран инженерный режим, и false - когда обычный
    connect(m_engineeringRadio, &QRadioButton::toggled, this, &Calculator::setEngineeringMode);

    setStyleSheet(STYLE_SHEET);
    setEngineeringMode(false);
}

// создаём кнопку, разрешаем ей растягиваться (нужно для "0" и "="
// на две клетки) и записываем вид кнопки в свойство kind для qss
QPushButton* Calculator::createButton(const QString& p_text, const QString& p_kind)
{
    QPushButton* button = new QPushButton(p_text);
    button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    button->setFocusPolicy(Qt::NoFocus);
    button->setProperty("kind", p_kind);
    return button;
}

// по нажатию вызывается digitClicked с этой цифрой
QPushButton* Calculator::addDigitButton(QGridLayout* p_grid, const QString& p_digit,
                                        int p_row, int p_column, int p_columnSpan)
{
    QPushButton* button = createButton(p_digit, "digit");
    connect(button, &QPushButton::clicked, this, [this, p_digit]() { digitClicked(p_digit); });
    p_grid->addWidget(button, p_row, p_column, 1, p_columnSpan);
    return button;
}

// по нажатию вызывается unaryOperationClicked с этой операцией
QPushButton* Calculator::addUnaryButton(QGridLayout* p_grid, const QString& p_text,
                                        const QString& p_operation, int p_row, int p_column)
{
    QPushButton* button = createButton(p_text, "function");
    connect(button, &QPushButton::clicked, this,
            [this, p_operation]() { unaryOperationClicked(p_operation); });
    p_grid->addWidget(button, p_row, p_column);
    return button;
}

// по нажатию вызывается binaryOperationClicked с этой операцией
QPushButton* Calculator::addBinaryButton(QGridLayout* p_grid, const QString& p_text,
                                         const QString& p_operation, int p_row, int p_column,
                                         const QString& p_kind)
{
    QPushButton* button = createButton(p_text, p_kind);
    connect(button, &QPushButton::clicked, this,
            [this, p_operation]() { binaryOperationClicked(p_operation); });
    p_grid->addWidget(button, p_row, p_column);
    return button;
}

// сетка 4 строки на 5 столбцов, как на рисунке из задания:
//   7 8 9 / sqrt
//   4 5 6 * 1/x
//   1 2 3 - =
//   0   . +
QGridLayout* Calculator::createNumberPanel()
{
    QGridLayout* grid = new QGridLayout;

    // цифры 1-9: в верхней строке 7 8 9, в нижней 1 2 3
    for (int digit = 1; digit <= 9; ++digit)
    {
        int row = 2 - (digit - 1) / 3;
        int column = (digit - 1) % 3;
        addDigitButton(grid, QString::number(digit), row, column);
    }
    addDigitButton(grid, "0", 3, 0, 2);   // ноль занимает две клетки

    QPushButton* pointButton = createButton(".", "digit");
    connect(pointButton, &QPushButton::clicked, this, &Calculator::pointClicked);
    grid->addWidget(pointButton, 3, 2);

    addBinaryButton(grid, "/", "/", 0, 3);
    addBinaryButton(grid, "*", "*", 1, 3);
    addBinaryButton(grid, "-", "-", 2, 3);
    addBinaryButton(grid, "+", "+", 3, 3);

    m_sqrtButton = addUnaryButton(grid, "sqrt", "sqrt", 0, 4);
    addUnaryButton(grid, "1/x", "1/x", 1, 4);

    QPushButton* equalButton = createButton("=", "equal");
    connect(equalButton, &QPushButton::clicked, this, &Calculator::equalClicked);
    grid->addWidget(equalButton, 2, 4, 2, 1);   // "=" занимает две строки

    return grid;
}

// панель 4 на 4, как на рисунке из задания:
//   sinh sin exp x²
//   cosh cos ln  x³
//   tanh tan log xʸ
//   n!   π   ∛x  ʸ√x
QWidget* Calculator::createEngineeringPanel()
{
    QWidget* panel = new QWidget;
    QGridLayout* grid = new QGridLayout(panel);
    grid->setContentsMargins(0, 0, 12, 0);   // отступ справа отделяет панель от цифр

    addUnaryButton(grid, "sinh", "sinh", 0, 0);
    addUnaryButton(grid, "sin", "sin", 0, 1);
    addUnaryButton(grid, "exp", "exp", 0, 2);
    addUnaryButton(grid, "x²", "x^2", 0, 3);

    addUnaryButton(grid, "cosh", "cosh", 1, 0);
    addUnaryButton(grid, "cos", "cos", 1, 1);
    addUnaryButton(grid, "ln", "ln", 1, 2);
    addUnaryButton(grid, "x³", "x^3", 1, 3);

    addUnaryButton(grid, "tanh", "tanh", 2, 0);
    addUnaryButton(grid, "tan", "tan", 2, 1);
    addUnaryButton(grid, "log", "log", 2, 2);
    addBinaryButton(grid, "xʸ", "x^y", 2, 3, "function");

    addUnaryButton(grid, "n!", "n!", 3, 0);

    QPushButton* piButton = createButton("π", "function");
    connect(piButton, &QPushButton::clicked, this, &Calculator::piClicked);
    grid->addWidget(piButton, 3, 1);

    addUnaryButton(grid, "∛x", "cbrt", 3, 2);
    addBinaryButton(grid, "ʸ√x", "yroot", 3, 3, "function");

    return panel;
}

// ---------------------------------------------------------- режимы

// прячем или показываем инженерную панель - ширина окна меняется сама,
// потому что у раскладки стоит SetFixedSize; меняем заголовок и надпись корня
void Calculator::setEngineeringMode(bool p_engineering)
{
    m_engineeringPanel->setVisible(p_engineering);
    m_sqrtButton->setText(p_engineering ? "√x" : "sqrt");
    setWindowTitle(p_engineering ? "Калькулятор (инженерный)" : "Калькулятор (обычный)");
}

// ---------------------------------------------------------- ввод числа

// если ждём новое число - начинаем его с этой цифры, иначе дописываем цифру в конец
void Calculator::digitClicked(const QString& p_digit)
{
    QString text = m_display->toPlainText();

    if (m_waitingForOperand || text == "0")
        text = p_digit;
    else if (text.length() < MAX_LENGTH)
        text += p_digit;

    m_display->setPlainText(text);
    m_waitingForOperand = false;
    m_newNumberEntered = true;
}

// добавляем точку, если её в числе ещё нет
void Calculator::pointClicked()
{
    if (m_waitingForOperand)
        m_display->setPlainText("0");

    QString text = m_display->toPlainText();
    if (!text.contains('.'))
        m_display->setPlainText(text + ".");

    m_waitingForOperand = false;
    m_newNumberEntered = true;
}

// --------------------------------------------------------------- операции

// если уже есть отложенная операция и введено второе число - сначала считаем её
// (так 2 + 3 * 4 считается по порядку: (2 + 3) * 4), потом запоминаем новую
void Calculator::binaryOperationClicked(const QString& p_operation)
{
    double operand = displayValue();

    if (!m_pendingOperation.isEmpty() && m_newNumberEntered)
    {
        if (!calculate(operand))
        {
            showError();
            return;
        }
        showNumber(m_result);
    }
    else if (m_pendingOperation.isEmpty())
    {
        m_result = operand;
    }

    // если второе число ещё не введено, операция просто заменяется (5 + * 2 = 10)
    m_pendingOperation = p_operation;
    m_waitingForOperand = true;
    m_newNumberEntered = false;
}

// считаем функцию от числа на дисплее. Ошибки (корень из отрицательного,
// деление на ноль, логарифм нуля) отдельно не проверяем: в этих случаях
// получается NaN или бесконечность, и showNumber покажет ошибку
void Calculator::unaryOperationClicked(const QString& p_operation)
{
    double x = displayValue();
    double result = 0.0;

    if (p_operation == "sqrt")
        result = std::sqrt(x);
    else if (p_operation == "1/x")
        result = 1.0 / x;
    else if (p_operation == "sin")       // углы в радианах
        result = std::sin(x);
    else if (p_operation == "cos")
        result = std::cos(x);
    else if (p_operation == "tan")
        result = std::tan(x);
    else if (p_operation == "sinh")
        result = std::sinh(x);
    else if (p_operation == "cosh")
        result = std::cosh(x);
    else if (p_operation == "tanh")
        result = std::tanh(x);
    else if (p_operation == "exp")
        result = std::exp(x);
    else if (p_operation == "ln")
        result = std::log(x);
    else if (p_operation == "log")       // десятичный логарифм
        result = std::log10(x);
    else if (p_operation == "x^2")
        result = x * x;
    else if (p_operation == "x^3")
        result = x * x * x;
    else if (p_operation == "cbrt")
        result = std::cbrt(x);
    else if (p_operation == "n!")
        result = factorial(x);

    if (!std::isfinite(result))
    {
        showError();
        return;
    }

    showNumber(result);
    m_waitingForOperand = true;
    m_newNumberEntered = true;
}

// досчитываем отложенную операцию и показываем результат
void Calculator::equalClicked()
{
    if (m_pendingOperation.isEmpty())
        return;

    if (!calculate(displayValue()))
    {
        showError();
        return;
    }

    showNumber(m_result);
    m_pendingOperation.clear();
    m_waitingForOperand = true;
    m_newNumberEntered = false;
}

// π ведёт себя как новое введённое число
void Calculator::piClicked()
{
    showNumber(PI);
    m_waitingForOperand = true;
    m_newNumberEntered = true;
}

// дисплей - "0", никаких отложенных операций
void Calculator::clearAll()
{
    m_display->setPlainText("0");
    m_result = 0.0;
    m_pendingOperation.clear();
    m_waitingForOperand = true;
    m_newNumberEntered = false;
}

// применяем отложенную операцию к m_result и правому числу
bool Calculator::calculate(double p_rightOperand)
{
    if (m_pendingOperation == "+")
    {
        m_result += p_rightOperand;
    }
    else if (m_pendingOperation == "-")
    {
        m_result -= p_rightOperand;
    }
    else if (m_pendingOperation == "*")
    {
        m_result *= p_rightOperand;
    }
    else if (m_pendingOperation == "/")
    {
        if (p_rightOperand == 0.0)
            return false;
        m_result /= p_rightOperand;
    }
    else if (m_pendingOperation == "x^y")
    {
        m_result = std::pow(m_result, p_rightOperand);
    }
    else if (m_pendingOperation == "yroot")
    {
        // корень степени y из x: x в степени 1/y
        if (p_rightOperand == 0.0)
            return false;

        // из отрицательного числа корень есть только нечётной степени: ∛-8 = -2
        bool isOddInteger = p_rightOperand == std::floor(p_rightOperand)
                         && static_cast<long long>(p_rightOperand) % 2 != 0;

        if (m_result < 0 && isOddInteger)
            m_result = -std::pow(-m_result, 1.0 / p_rightOperand);
        else
            m_result = std::pow(m_result, 1.0 / p_rightOperand);
    }

    // NaN или бесконечность - значит, посчитать нельзя
    return std::isfinite(m_result);
}

// ----------------------------------------------------------------- дисплей

// 12 значащих цифр: 0.1 + 0.2 покажется как 0.3, а не 0.30000000000000004
void Calculator::showNumber(double p_value)
{
    if (!std::isfinite(p_value))
    {
        showError();
        return;
    }

    if (p_value == 0.0)
        p_value = 0.0;   // убираем "-0"

    m_display->setPlainText(QString::number(p_value, 'g', 12));
}

// пишем "Ошибка" и сбрасываем отложенную операцию
void Calculator::showError()
{
    m_display->setPlainText("Ошибка");
    m_result = 0.0;
    m_pendingOperation.clear();
    m_waitingForOperand = true;
    m_newNumberEntered = false;
}

// текст дисплея переводим в число; для "Ошибка" toDouble вернёт 0
double Calculator::displayValue() const
{
    return m_display->toPlainText().toDouble();
}
