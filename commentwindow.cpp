#include "commentwindow.h"
#include "mainwindow.h"
#include "ui_commentwindow.h"
#include "globalsetting.h"
#include "utilitytools.h"
#include <QInputDialog>
#include <QLabel>
#include <QMenu>
#include <QMessagebox>
#include <QtNetwork>
#include "registerandlogin.h"

CommentWindow* CommentWindow::MyInstance=NULL;
CommentWindow::CommentWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CommentWindow)
{
    if(MyInstance)
        throw 0;
    MyInstance=this;
    ui->setupUi(this);
    NetworkM=new QNetworkAccessManager();
    //setWindowFlag(Qt::FramelessWindowHint);
    SuperWndProc=(WNDPROC)SetWindowLongPtr((HWND)this->winId(),GWLP_WNDPROC,(LONG_PTR)&CommentWindow_MyWndProc);
    connect(this,&CommentWindow::WindowMoved,this,&CommentWindow::onWindowMoved,Qt::UniqueConnection);
    GetComment(MainWindow::MyInstance->GetCurrentMusic(),0);
}

LRESULT CALLBACK CommentWindow_MyWndProc(HWND wnd,UINT message,WPARAM wParam,LPARAM lParam)
{
    if(message==WM_MOVE)
    {
        LRESULT returnv=CommentWindow::MyInstance->SuperWndProc(wnd,message,wParam,lParam);
        emit CommentWindow::MyInstance->WindowMoved();
        return returnv;
    }
    return CommentWindow::MyInstance->SuperWndProc(wnd,message,wParam,lParam);
}

CommentWindow::~CommentWindow()
{
    delete ui;
    MyInstance=NULL;
}

void CommentWindow::onWindowMoved()
{
    MainWindow::MyInstance->move(pos().x()-MainWindow::MyInstance->width()-2,pos().y());
}

void CommentWindow::SubmitComment(int MusicID, int TargetComment)
{
    QString strInput = QInputDialog::getMultiLineText(nullptr,u8"输入评论",u8"想说些什么？");
    QByteArray byteArrayUTF8;
    QByteArray byteArrayPercentEncoded = byteArrayUTF8.toPercentEncoding();
    QByteArray postData;
    postData.append(QString("UserID=%1&UserToken=%2&MusicID=%3&ReplyCommentID=%4&CommentText=")
                    .arg(RegisterAndLogin::GetUserID())
                    .arg(RegisterAndLogin::GetGlobalToken())
                    .arg(MusicID)
                    .arg(TargetComment));
    postData.append(strInput.toHtmlEscaped());
    QNetworkRequest tmpRequest=QNetworkRequest(QString("https://do.jackmyth.cn/MyMusicWebside/SubmitComment.php"));
    //tmpRequest.setHeader(QNetworkRequest::ContentTypeHeader,"text/html;charset=utf-8");
    QNetworkReply* NetR = NetworkM->post(tmpRequest,postData);
    disconnect(NetworkM,&QNetworkAccessManager::finished,0,0);
    connect(NetR,&QNetworkReply::finished,this,[=]()
    {
        disconnect(NetworkM,&QNetworkAccessManager::finished,0,0);
        NetR->deleteLater();
        QJsonObject resUlt =QJsonDocument::fromJson(NetR->readAll()).object();
        if(resUlt.find("Result").value().toString()=="Success")
        {
            CommentWindow::MyInstance->GetComment(MainWindow::MyInstance->GetCurrentMusic(),0);
            return;
        }
        else
        {
            QMessageBox::warning(nullptr,u8"评论失败",resUlt.find("ErrorMsg").value().toString(u8"未知错误"));
            return;
        }
    });
}

void CommentWindow::AddComment(QJsonObject JsonObject)
{
    QWidget* widget=new QWidget();
    QLabel* Icon=new QLabel(widget);
    Icon->resize(50,50);
    Icon->move(0,0);
    QLabel* MainContent=new QLabel(widget);
    MainContent->setMaximumWidth(this->width()-51);
    MainContent->setMinimumWidth(this->width()-51);
    MainContent->setMinimumHeight(10);
    //MainContent->resize(MainContent->height(),MainContent->maximumWidth());
    MainContent->setAlignment(Qt::AlignTop);
    MainContent->move(50,0);
    MainContent->setWordWrap(true);
    //QJsonObject user= JsonObject.find("user").value().toObject();
    //QString AvatarURL=user.find("avatarUrl").value().toString();
    QString NickName=JsonObject.find("UserName").value().toString();
    QString Content=JsonObject.find("CommentText").value().toString();
    MainContent->setText(NickName+QString(":")+ Content);
    MainContent->adjustSize();
    MainContent->resize(0,MainContent->height()+20);
    bool IsReply=!JsonObject.find("ReplyUserName").value().isNull();
    QString ReplyName =JsonObject.find("ReplyUserName").value().toString();
    QString ReplyComment =JsonObject.find("ReplyCommentText").value().toString();
    int lastY=MainContent->height()+5;
    if(IsReply)
    {
        QLabel* subContentLabel=new QLabel(widget);
        subContentLabel->setMaximumWidth(this->width()-61);
        subContentLabel->setMinimumWidth(this->width()-61);
        subContentLabel->setMinimumHeight(10);
        subContentLabel->setWordWrap(true);
        //subContentLabel->resize(subContentLabel->height(),subContentLabel->maximumWidth());
        subContentLabel->setAlignment(Qt::AlignTop);
        subContentLabel->setStyleSheet("background-color: rgb(238, 238, 238);");
        QString subNickName=ReplyName;
        QString subContent=ReplyComment;
        subContentLabel->setText(subNickName+ QString(":")+ subContent);
        subContentLabel->adjustSize();
        subContentLabel->resize(0,subContentLabel->height()+20);
        subContentLabel->move(60,lastY);
        lastY+=subContentLabel->height()+5;
    }
    auto ListItem=new QListWidgetItem(ui->CommentList);
    CommentIDs.push_back(JsonObject.find("CommentID").value().toInt());
    ui->CommentList->addItem(ListItem);
    ui->CommentList->setItemWidget(ListItem,widget);
    ListItem->setSizeHint(QSize(0,lastY>60?lastY:60));
}

void CommentWindow::GetComment(MusicInfomation Music,int Page)
{
    if(!Music.ID)
    {
        ui->progressBar->hide();
        ui->CommentList->clear();
        return;
    }
    ui->progressBar->show();
    CurrentPage=Page;
    disconnect(NetworkM,&QNetworkAccessManager::finished,0,0);
    connect(NetworkM,&QNetworkAccessManager::finished,this,[=](QNetworkReply* CommentResult)
    {
        disconnect(NetworkM,&QNetworkAccessManager::finished,0,0);
        ui->progressBar->hide();
        for(int i=0;i<SameLabelCollectionInstance.size();i++)
            SameLabelCollectionInstance[i]->Clear();
        SameLabelCollectionInstance.clear();
        ui->CommentList->clear();
        CommentIDs.clear();
        QJsonArray Comments=QJsonDocument::fromJson(CommentResult->readAll()).array();
        if(Comments.size()<=1)
            ui->CommentList->addItem(u8"这首歌还没有人评论呢。");
        else
        {
            ui->CommentList->addItem(u8"最新评论:");
            for(int i=1;i<Comments.size();i++)
            {
                AddComment(Comments[i].toObject());
            }
        }
        if(GlobalSetting::CommentLimit*Page<Comments[0].toInt())
            ui->NextButton->setEnabled(true);
        else
            ui->NextButton->setEnabled(false);
        if(Page>0)
            ui->PreButton->setEnabled(true);
        else
            ui->PreButton->setEnabled(false);
        CommentResult->deleteLater();
    },Qt::UniqueConnection);
    NetworkM->get(QNetworkRequest(QString("https://do.jackmyth.cn/MyMusicWebside/GetComments.php?MusicID=%1&Begin=%2&End=%3")
                                  .arg(Music.ID).arg(GlobalSetting::CommentLimit*Page).arg(GlobalSetting::CommentLimit*(Page+1))));
}

void CommentWindow::Logined()
{
    ui->label->setText(QString(u8"已作为“%1”登陆").arg(CurrentUserName));
    ui->LoginAndComment->setText(u8"评论");
}

void CommentWindow::on_NextButton_clicked()
{
    GetComment(MainWindow::MyInstance->GetCurrentMusic(),CurrentPage+1);
}

void CommentWindow::on_PreButton_clicked()
{
    GetComment(MainWindow::MyInstance->GetCurrentMusic(),CurrentPage-1);
}

void CommentWindow::on_RefreshButton_clicked()
{
    GetComment(MainWindow::MyInstance->GetCurrentMusic(),CurrentPage);
}

void CommentWindow::on_LoginAndComment_clicked()
{
    if(RegisterAndLogin::GetGlobalToken()=="")
    {
        RegisterAndLogin::Show();
    }
    else
    {
        SubmitComment(MainWindow::MyInstance->GetCurrentMusic().ID,0);
    }
}

void CommentWindow::on_CommentList_customContextMenuRequested(const QPoint &pos)
{
    if(ui->CommentList->currentRow()-1>=0&&ui->CommentList->currentRow()-1<CommentIDs.size())
    {
        QMenu menu;
        menu.addAction(u8"评论这条评论",[=]()
        {
            SubmitComment(MainWindow::MyInstance->GetCurrentMusic().ID,CommentIDs[ui->CommentList->currentRow()-1]);
        });
        menu.exec(QCursor::pos());
    }
}
