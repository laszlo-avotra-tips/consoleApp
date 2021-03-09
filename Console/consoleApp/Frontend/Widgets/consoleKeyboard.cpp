#include "consoleKeyboard.h"
#include "ui_consoleKeyboard.h"

#include <QDebug>


ConsoleKeyboard::ConsoleKeyboard(const ParameterType &param, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleKeyboard)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    auto paramName = ui->labelParam;
    auto paramValue = ui->lineEditParam;

    /*
     * paramName - the name of the parameter to be edited
     */
    paramName->setText(param[0]);

    /*
     * paramValue - the value of the parameter
     */
    paramValue->setText(param[1]);

    /*
     * if there is a third parameter passed in the constructor it applies to the
     * action takem on keyboard enter
     */
    if(param.size() > 2){
        auto* enterButton = ui->pushButton_enter;
        auto actionOnEnter = param[2];
        if(!actionOnEnter.isEmpty()){
            qDebug() << __FUNCTION__ << " " << actionOnEnter;
            enterButton->setText(actionOnEnter);
        }
    }
    ui->pushButton_enter->setEnabled(false);

    /*
     * connect the UI widgets to this classes methods
     */
    auto* enterButton = ui->pushButton_enter;
    connect(enterButton, &QPushButton::clicked, this, &QDialog::accept);

    auto deleteButton = ui->pushButton_delete;
    connect(deleteButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleDelete);

    auto spaceButton = ui->pushButton_space;
    connect(spaceButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleSpace);

    auto capsLockButton = ui->pushButton_capsLock;
    connect(capsLockButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleCapsLock);

    auto leftShiftButton = ui->pushButton_shiftLeft;
    connect(leftShiftButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleLeftShift);

    auto rightShiftButton = ui->pushButton_shiftRight;
    connect(rightShiftButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleRightShift);

    /*
     * the keyboard buttons are aranged in containers
     */
    initButtonContainers();

    connectNumbers();
    connectOtherThanLetterOrNumber();
    connectLetters();

    connect(this, &ConsoleKeyboard::numberClicked, this, &ConsoleKeyboard::handleNumbersAndOthers);
    connect(this, &ConsoleKeyboard::otherThanLetterOrNumberClicked, this, &ConsoleKeyboard::handleNumbersAndOthers);
    connect(this, &ConsoleKeyboard::letterClicked, this, &ConsoleKeyboard::handleLetters);
}

ConsoleKeyboard::~ConsoleKeyboard()
{
    delete ui;
}

QString ConsoleKeyboard::value()
{
    return ui->lineEditParam->text();
}

void ConsoleKeyboard::handleDelete()
{
    auto* target = ui->lineEditParam;
    auto param = target->text();
    if(!param.isEmpty()){
       int lastPosition = param.size() - 1;
       target->setText(param.remove(lastPosition,1));
    }
    ui->lineEditParam->setFocus();
    highlightEnter();

}

void ConsoleKeyboard::handleSpace()
{
    const QString val = ui->lineEditParam->text() + QString(" ");
    ui->lineEditParam->setText(val);
    ui->lineEditParam->setFocus();
}

void ConsoleKeyboard::handleNumbersAndOthers(const QString& number)
{
    auto stringList = number.split("\n");
    if(stringList.size() == 2){
        if(m_isLowCap){
            const QString val = ui->lineEditParam->text() + stringList[1];
            ui->lineEditParam->setText(val);
        } else {
            const QString val = ui->lineEditParam->text() + stringList[0];
            ui->lineEditParam->setText(val);
            toggleCap();
        }
    } else if(stringList.size() == 1){
        const QString val = ui->lineEditParam->text() + stringList[0];
        ui->lineEditParam->setText(val);
    }
    ui->lineEditParam->setFocus();
    highlightEnter();
}

void ConsoleKeyboard::handleLetters(const QString &letter)
{
    const QString val = ui->lineEditParam->text() + letter;
    ui->lineEditParam->setText(val);
    ui->lineEditParam->setFocus();
    if(m_isShift){
        toggleCap();
        m_isShift = false;
    }
    highlightEnter();
}

void ConsoleKeyboard::initButtonContainers()
{
    const ButtonContainer numbers{
        ui->pushButton_zero,
        ui->pushButton_one,
        ui->pushButton_two,
        ui->pushButton_three,

        ui->pushButton_four,
        ui->pushButton_five,
        ui->pushButton_six,
        ui->pushButton_seven,

        ui->pushButton_eight,
        ui->pushButton_nine,
    };

    m_numberButtons = numbers;

    const ButtonContainer otherThanLetterOrNumberContainer{
        ui->pushButton_coma,
        ui->pushButton_dot,

        ui->pushButton_minus,
        ui->pushButton_equal,
        ui->pushButton_angleLeft,
        ui->pushButton_angleRight,

        ui->pushButton_slash,
        ui->pushButton_backSlash,
        ui->pushButton_quote,

    };

    m_otherThanLetterOrNumberContainer = otherThanLetterOrNumberContainer;

    const ButtonContainer letters{
        ui->pushButton_a,
        ui->pushButton_b,
        ui->pushButton_c,
        ui->pushButton_d,

        ui->pushButton_e,
        ui->pushButton_f,
        ui->pushButton_g,
        ui->pushButton_h,

        ui->pushButton_i,
        ui->pushButton_j,
        ui->pushButton_k,
        ui->pushButton_l,

        ui->pushButton_m,
        ui->pushButton_n,
        ui->pushButton_o,
        ui->pushButton_p,

        ui->pushButton_q,
        ui->pushButton_r,
        ui->pushButton_s,
        ui->pushButton_t,

        ui->pushButton_u,
        ui->pushButton_v,
        ui->pushButton_w,
        ui->pushButton_x,

        ui->pushButton_y,
        ui->pushButton_z,
    };

    m_letterButtons = letters;
}

void ConsoleKeyboard::connectNumbers()
{
    for(auto* button : m_numberButtons){
        connect(
            button,
            &QPushButton::clicked,
            [this, button]
            {
                numberClicked(button->text());
            }
        );
    }
}

void ConsoleKeyboard::connectLetters()
{
    for(auto* button : m_letterButtons){
        connect(
            button,
            &QPushButton::clicked, [this, button]
            {
                letterClicked(button->text());
            }
        );
    }
}

void ConsoleKeyboard::connectOtherThanLetterOrNumber()
{
    for(auto* button : m_otherThanLetterOrNumberContainer){
        connect(
            button,
            &QPushButton::clicked,
            [this, button]
            {
                otherThanLetterOrNumberClicked(button->text());
            }
        );
    }
}

void ConsoleKeyboard::toLowCap()
{
    for( auto* letterButton : m_letterButtons){
        auto letter = letterButton->text();
        auto lowerCase = letter.toLower();
        letterButton->setText(lowerCase);
        m_isLowCap = true;
    }
}

void ConsoleKeyboard::toHighCap()
{
    for( auto* letterButton : m_letterButtons){
        auto letter = letterButton->text();
        auto upperCase = letter.toUpper();
        letterButton->setText(upperCase);
        m_isLowCap= false;
    }
}

void ConsoleKeyboard::toggleCap()
{
    if(m_isLowCap){
        toHighCap();
    }else{
        toLowCap();
    }
}

void ConsoleKeyboard::buttonToUpper(QPushButton *button)
{
    auto txt = button->text();
    button->setText(txt.toUpper());
}

void ConsoleKeyboard::buttonToLower(QPushButton *button)
{
    auto txt = button->text();
    button->setText(txt.toLower());
}

void ConsoleKeyboard::highlightEnter()
{
    if(!ui->lineEditParam->text().isEmpty()){
        ui->pushButton_enter->setStyleSheet("background-color: rgb(245,196,0); color: black");
        ui->pushButton_enter->setEnabled(true);
    } else {
        //background-color: #343434;
        ui->pushButton_enter->setStyleSheet("background-color: #343434; color: black");
        ui->pushButton_enter->setEnabled(false);
    }
}

void ConsoleKeyboard::handleCapsLock(bool checked)
{
    if(checked){
        buttonToUpper(ui->pushButton_capsLock);
        toHighCap();
    } else {
        buttonToLower(ui->pushButton_capsLock);
        toLowCap();
    }
    ui->lineEditParam->setFocus();
}

void ConsoleKeyboard::handleLeftShift()
{
    handleRightShift();
}

void ConsoleKeyboard::handleRightShift()
{
    if(m_isLowCap){
        toggleCap();
        m_isShift = true;
    }
    ui->lineEditParam->setFocus();
}
