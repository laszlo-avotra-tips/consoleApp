#include "consoleKeyboard.h"
#include "ui_consoleKeyboard.h"


ConsoleKeyboard::ConsoleKeyboard(const ParameterType &param, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConsoleKeyboard)
{
    ui->setupUi(this);

    setWindowFlags(Qt::SplashScreen);

    auto paramLabel = ui->labelParam;
    auto paramLineEdit = ui->lineEditParam;

    paramLabel->setText(param[0]);
    paramLineEdit->setText(param[1]);

    auto* enterButton = ui->pushButton_enter;
    connect(enterButton, &QPushButton::clicked, this, &QDialog::accept);

    auto deleteButton = ui->pushButton_delete;
    connect(deleteButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleDelete);

    auto spaceButton = ui->pushButton_space;
    connect(spaceButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleSpace);

    auto capsLockButton = ui->pushButton_capsLock;
    connect(capsLockButton, &QPushButton::clicked, this, &ConsoleKeyboard::handleCapsLock);

    initButtonContainers();

    initNumbers();
    connect(this, &ConsoleKeyboard::numberClicked, this, &ConsoleKeyboard::handleNumbers);

    initLetters();
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
}

void ConsoleKeyboard::handleSpace()
{
    const QString val = ui->lineEditParam->text() + QString(" ");
    ui->lineEditParam->setText(val);
    ui->lineEditParam->setFocus();
}

void ConsoleKeyboard::handleNumbers(const QString& addText)
{
    auto stringList = addText.split("\n");
    if(stringList.size() == 2){
        const QString val = ui->lineEditParam->text() + stringList[1];
        ui->lineEditParam->setText(val);
    } else if(stringList.size() == 1){
        const QString val = ui->lineEditParam->text() + stringList[0];
        ui->lineEditParam->setText(val);
    }
    ui->lineEditParam->setFocus();
}

void ConsoleKeyboard::handleLetters(const QString &text)
{
    const QString val = ui->lineEditParam->text() + text;
    ui->lineEditParam->setText(val);
    ui->lineEditParam->setFocus();
    if(m_isShift){
        toggleCap();
        m_isShift = false;
    }
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
        ui->pushButton_coma,
        ui->pushButton_dot,

        ui->pushButton_minus,
        ui->pushButton_equal,
        ui->pushButton_angleLeft,
        ui->pushButton_angleRight,

        ui->pushButton_slash,
        ui->pushButton_backSlash,
    };

    m_numberButtons = numbers;

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

void ConsoleKeyboard::initNumbers()
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

void ConsoleKeyboard::initLetters()
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

void ConsoleKeyboard::pushButtonEnabled(QPushButton *button)
{
    auto txt = button->text();
    button->setText(txt.toUpper());
}

void ConsoleKeyboard::pushButtonDisabled(QPushButton *button)
{
    auto txt = button->text();
    button->setText(txt.toLower());
}

void ConsoleKeyboard::handleCapsLock(bool checked)
{
    if(checked){
        pushButtonEnabled(ui->pushButton_capsLock);
        toHighCap();
    } else {
        pushButtonDisabled(ui->pushButton_capsLock);
        toLowCap();
    }
    ui->lineEditParam->setFocus();
}

void ConsoleKeyboard::on_pushButton_shiftLeft_clicked()
{
    on_pushButton_shiftRight_clicked();
}

void ConsoleKeyboard::on_pushButton_shiftRight_clicked()
{
    if(m_isLowCap){
        toggleCap();
        m_isShift = true;
    }
    ui->lineEditParam->setFocus();
}
