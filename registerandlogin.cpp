#include "commentwindow.h"
#include "registerandlogin.h"
#include "ui_registerandlogin.h"
#include <QMessagebox>
#include <QtNetwork>

QString RegisterAndLogin::GlobalToken="";
QString RegisterAndLogin::UserID="";

RegisterAndLogin* RegisterAndLogin::MyInstance=nullptr;

void RegisterAndLogin::Show()
{
    if(!MyInstance)
        MyInstance=new RegisterAndLogin();
    MyInstance->setWindowModality(Qt::ApplicationModal);
    MyInstance->show();
}

RegisterAndLogin::RegisterAndLogin(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::RegisterAndLogin)
{
    ui->setupUi(this);
}

RegisterAndLogin::~RegisterAndLogin()
{
    delete ui;
}

QString RegisterAndLogin::GetGlobalToken()
{
    return GlobalToken;
}

QString RegisterAndLogin::GetUserID()
{
    return UserID;
}

void RegisterAndLogin::on_radioButton_2_clicked()
{
    ui->lineEdit_2->hide();
    ui->lineEdit_4->hide();
    ui->pushButton_2->setText(u8"登陆");
}

void RegisterAndLogin::on_radioButton_clicked()
{
    ui->lineEdit_2->show();
    ui->lineEdit_4->show();
    ui->pushButton_2->setText(u8"注册");
}

void RegisterAndLogin::on_pushButton_2_clicked()
{
    if(ui->radioButton->isChecked())
    {
        //注册
        if(ui->lineEdit_3->text()!=ui->lineEdit_4->text())
        {
            QMessageBox::information(nullptr,u8"密码不一致！",u8"错误");
            return;
        }
        QString PasswordMD5=QCryptographicHash::hash(ui->lineEdit_3->text().toLatin1(),QCryptographicHash::Md5).toHex();;
        QNetworkAccessManager* tmpNetworkManager=new QNetworkAccessManager();
        QByteArray postData=QString("UserID=%1&UserName=%2&PasswordMD5=%3")
                .arg(ui->lineEdit->text())
                .arg(ui->lineEdit_2->text())
                .arg(PasswordMD5).toLatin1();
        QNetworkReply* tmpReply= tmpNetworkManager->post(QNetworkRequest(
            QString("https://do.jackmyth.cn/MyMusicWebside/Register.php")),postData);
        connect(tmpReply,&QNetworkReply::finished,this,[=]()
        {
            tmpReply->deleteLater();
            tmpNetworkManager->deleteLater();
            QJsonObject resUlt =QJsonDocument::fromJson(tmpReply->readAll()).object();
            if(resUlt.find("Result").value().toString()=="Success")
            {
                QMessageBox::information(nullptr,u8"注册成功！",u8"欢迎加入MyMusicPlayer");
                return;
            }
            else
            {
                QMessageBox::warning(nullptr,u8"注册失败",resUlt.find("ErrMsg").value().toString(u8"未知错误"));
                return;
            }
        });
    }
    else
    {
        QString PasswordMD5=QCryptographicHash::hash(ui->lineEdit_3->text().toLatin1(),QCryptographicHash::Md5).toHex();;
        QNetworkAccessManager* tmpNetworkManager=new QNetworkAccessManager();
        QByteArray postData=QString("UserID=%1&PassMD5=%2")
                .arg(ui->lineEdit->text())
                .arg(PasswordMD5).toLatin1();
        QNetworkReply* tmpReply= tmpNetworkManager->post(QNetworkRequest(
            QString("https://do.jackmyth.cn/MyMusicWebside/Login.php")),postData);
        QString tmpUserID=ui->lineEdit->text();
        connect(tmpReply,&QNetworkReply::finished,this,[=]()
        {
            tmpReply->deleteLater();
            tmpNetworkManager->deleteLater();
            QJsonObject resUlt =QJsonDocument::fromJson(tmpReply->readAll()).object();
            if(resUlt.find("LoginResult").value().toString()=="LoginSuccess")
            {
                QMessageBox::information(nullptr,u8"登陆成功！",u8"欢迎回来");
                GlobalToken=resUlt.find("NewToken").value().toString();
                UserID=tmpUserID;
                CommentWindow::CurrentUserName=resUlt.find("Result").value().toString();
                CommentWindow::MyInstance->Logined();
                close();
                return;
            }
            else
            {
                QMessageBox::warning(nullptr,u8"登陆失败",resUlt.find("ErrorMsg").value().toString(u8"未知错误"));
                return;
            }
        });
    }
}
