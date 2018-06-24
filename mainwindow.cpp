﻿#include "mainwindow.h"
#include "searchwindow.h"
#include "ui_mainwindow.h"
#include "globalsetting.h"
#include "utilitytools.h"
#include "commentwindow.h"
#include "settingdialog.h"
#include <QMenu>
#include <QMediaService>
#include <QMediaPlaylist>
#include <QMessageBox>
#include <QMediaPlayerControl>
#include <QWidgetItem>

MainWindow* MainWindow::MyInstance=NULL;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    if(MyInstance)
        throw 0;
    ui->setupUi(this);
    MusicPlayerCore=new QMediaPlayer(0,QMediaPlayer::StreamPlayback);
    NetworkMusic=new QNetworkAccessManager();
    NetworkM=new QNetworkAccessManager();
    MyInstance=this;
    DownloadManagerInstance=new DownloadManager();
    setAttribute(Qt::WA_DeleteOnClose);
    //链接信号和槽
    connect(MusicPlayerCore,&QMediaPlayer::stateChanged,this,&MainWindow::onMusicPlayerStateChanged,Qt::UniqueConnection);
    connect(MusicPlayerCore,&QMediaPlayer::positionChanged,this,&MainWindow::onMusicPlayerPositionChanged,Qt::UniqueConnection);
    connect(MusicPlayerCore,&QMediaPlayer::mediaStatusChanged,this,&MainWindow::mediaStatusChanged,Qt::UniqueConnection);
    //connect(MusicPlayerCore,&QMediaPlayer::bufferStatusChanged,this,&MainWindow::onBufferStateChanged,Qt::UniqueConnection);
    connect(ui->ShowOriginLyric,&QPushButton::clicked,this,&MainWindow::onLyricSettingChanged,Qt::UniqueConnection);
    connect(ui->ShowTranslatedLyric,&QPushButton::clicked,this,&MainWindow::onLyricSettingChanged,Qt::UniqueConnection);
    SuperWndProc=(WNDPROC)SetWindowLongPtr((HWND)this->winId(),GWLP_WNDPROC,(LONG_PTR)&MainWindow_MyWndProc); //重新设置WndProc，用以接受WM_MOVE消息
    connect(this,&MainWindow::WindowMoved,this,&MainWindow::onWindowMoved,Qt::UniqueConnection);
}

LRESULT CALLBACK MainWindow_MyWndProc(HWND wnd,UINT message,WPARAM wParam,LPARAM lParam) //WndProc
{
    if(message==WM_MOVE)
    {
        LRESULT returnv=MainWindow::MyInstance->SuperWndProc(wnd,message,wParam,lParam);
        emit MainWindow::MyInstance->WindowMoved();
        return returnv;
    }
    return MainWindow::MyInstance->SuperWndProc(wnd,message,wParam,lParam);//调用原来的WndProc
}

MainWindow::~MainWindow()
{
    delete ui;
    delete MusicPlayerCore;
    delete NetworkM;
    MyInstance=NULL;
    exit(0); //主窗体关闭后退出
}

void MainWindow::onWindowMoved()  //窗体移动事件
{
    if(CommentWindow::MyInstance)
    {
        CommentWindow::MyInstance->raise();
        CommentWindow::MyInstance->move(pos().x()+this->width()+2,pos().y());
    }
}

void MainWindow::onMusicPlayerStateChanged(QMediaPlayer::State state) //歌曲状态改变事件
{
    switch(state)
    {
    case QMediaPlayer::PlayingState:
        ui->PlayButton->setStyleSheet("border-image: url(:/Resources/pause.png);");
        break;
    case QMediaPlayer::PausedState:
    case QMediaPlayer::StoppedState:
        ui->PlayButton->setStyleSheet("border-image: url(:/Resources/play.png);");
        break;
    }
}

void MainWindow::onMusicPlayerPositionChanged(qint64 position)  //音乐进度改变事件，频率不高，大概一秒触发一次
{
    //qDebug()<<MusicPlayerCore->bufferStatus();
    if(!SliderPressing)
        ui->ProgressSlider->setValue((float)position/MusicPlayerCore->duration()*ui->ProgressSlider->maximum());
    int Lrcline;
    if(LRCParserInstance.GetLyricLineIndex(position,Lrcline))
    {
        ui->LyricList->scrollToItem(ui->LyricList->item(Lrcline),QListWidget::PositionAtCenter);
        ui->LyricList->setCurrentRow(Lrcline);
    }
}

void MainWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status) //在歌曲播放完之后自动播放列表中的下一个
{
    //return;
    if(status==QMediaPlayer::EndOfMedia)
    {
        if(!MusicCollection.size())
            return;
        CurrentMusic=(CurrentMusic+(CurrentMusic<0?MusicCollection.size():1))%MusicCollection.size();
        __CheckAndPlayMusic(MusicCollection[CurrentMusic]);
        __UpdatePlaylist();
    }
}

void MainWindow::onLyricSettingChanged()  //歌词切换按钮，处理当前的歌词状态
{
    if(ui->ShowOriginLyric->isChecked())
        if(ui->ShowTranslatedLyric->isChecked())
            LyricToShow=LRCParser::LyricType::Mixed;
        else
            LyricToShow=LRCParser::LyricType::Origin;
    else
        if(ui->ShowTranslatedLyric->isChecked())
            LyricToShow=LRCParser::LyricType::Translation;
        else
        {
            //两个全都弹起时自动按下“原歌词”
            //disconnect(ui->ShowOriginLyric,&QPushButton::clicked,this,&MainWindow::onLyricSettingChanged);
            ui->ShowOriginLyric->setChecked(true);
            //connect(ui->ShowOriginLyric,&QPushButton::clicked,this,&MainWindow::onLyricSettingChanged,Qt::UniqueConnection);
            //ui->ShowTranslatedLyric->setChecked(true);
            return;
        }
    ui->LyricList->clear();
    auto lrcD= LRCParserInstance.GetLyricData(LyricToShow);
    for(int i=0;i<lrcD.size();i++)
        ui->LyricList->addItem(lrcD[i].Line);
}

void MainWindow::PlayMusic()
{
    MusicPlayerCore->play();
}

void MainWindow::PauseMusic()
{
    MusicPlayerCore->pause();
}

void MainWindow::PlayMusic(MusicInfomation MusicToPlay,bool AddToPlaylist)
{
    if(AddToPlaylist)
    {
        if(CurrentMusic<0)
            if(MusicCollection.size()==0)
                CurrentMusic=0;
            else
                CurrentMusic+=MusicCollection.size();
        if(MusicCollection.indexOf(MusicToPlay)<0)
        {
            MusicCollection.insert(MusicCollection.begin()+CurrentMusic,MusicToPlay);
            __UpdatePlaylist();
        }
    }
    else
        CurrentMusic-=MusicCollection.size();
    __CheckAndPlayMusic(MusicToPlay);
}

void MainWindow::PlayMusicNext(MusicInfomation MusicToPlay)
{
    //if(MusicCollection.count(tmpMusicinfo)>0)
    MusicCollection.removeAll(MusicToPlay);
    if(CurrentMusic<0)
    {
        if(!MusicCollection.size())
            MusicCollection.insert(MusicCollection.begin(),MusicToPlay);
        else
        {
            MusicCollection.insert(MusicCollection.begin()+CurrentMusic+MusicCollection.size(),MusicToPlay);
            CurrentMusic--;
        }
    }
    else
        MusicCollection.insert(MusicCollection.begin()+CurrentMusic+1,MusicToPlay);
    __UpdatePlaylist();
}

void MainWindow::PlayNext()
{
    CurrentMusic=(CurrentMusic+(CurrentMusic<0?MusicCollection.size():1))%MusicCollection.size();
    __CheckAndPlayMusic(MusicCollection[CurrentMusic]);
    __UpdatePlaylist();
}

void MainWindow::PlayPre()
{
    CurrentMusic=(CurrentMusic+(CurrentMusic<0?MusicCollection.size():MusicCollection.size()-1))%MusicCollection.size();
    __CheckAndPlayMusic(MusicCollection[CurrentMusic]);
    __UpdatePlaylist();
}

void MainWindow::__CheckAndPlayMusic(MusicInfomation MusicToPlay)
{
    ui->Music_Label->setText(MusicToPlay.Name);
    ui->AlbumName_Label->setText(u8"专辑:" + MusicToPlay.Album.Name);
    ui->ArtistName_Label->setText(u8"艺术家:" +MusicToPlay.ArtistsName);
    ui->Small_Music_Artist->setText(MusicToPlay.Name+" - "+MusicToPlay.ArtistsName);
    new GetPictureFromURL(MusicToPlay.Album.PicURL,this,[this](QPixmap pixmap)
    {
        ui->MusicAvater->setPixmap(pixmap);
    });//获得专辑封面
    CurrentMusicInfo=MusicToPlay;
    QString Md5Hash= QCryptographicHash::hash(MusicToPlay.URL.toLocal8Bit(),QCryptographicHash::Md5).toHex();
    QFile LocalFile(GlobalSetting::CacheDir +Md5Hash);
    //MusicPlayerCore->service()->requestControl<QMediaPlayerControl>().setProperty("mediaDownloadEnabled", true);
    if(LocalFile.exists())
    {
        qDebug()<<(GlobalSetting::CacheDir +Md5Hash);
        MusicPlayerCore->setMedia(QMediaContent(QUrl::fromLocalFile(GlobalSetting::CacheDir +Md5Hash)));
        if(QFile(GlobalSetting::DownloadDir +UtitlityTools::GetValidFileName(MusicToPlay.Name + " - " + MusicToPlay.ArtistsName+".mp3")).exists())
            ui->Download_Button->setText(u8"已下载");
        else
            ui->Download_Button->setText(u8"已缓存");
        goto beginPlay;
    }
    ui->Download_Button->setText(u8"下载");
    MusicPlayerCore->setMedia(QMediaContent(MusicToPlay.URL));
    if(GlobalSetting::AutoCache)
        BeginCache(MusicToPlay);
beginPlay:
    MusicPlayerCore->play();
    /*if(CommentWindow::MyInstance&&CurrentMusicInfo.ID) //获取评论
        CommentWindow::MyInstance->GetComment(CurrentMusicInfo,0);*/
    LRCParserInstance.ClearLyric();
    ui->LyricList->clear();
    disconnect(NetworkM,&QNetworkAccessManager::finished,0,0);
    if(MusicToPlay.LyricID)
    {
        connect(NetworkM,&QNetworkAccessManager::finished,this,[=](QNetworkReply* LyricResult) //尝试获取歌词
        {
            QJsonObject LrcJson=QJsonDocument::fromJson(LyricResult->readAll()).object();
            QString LyricStr= LrcJson.find("LyricSource").value().toString();
            QString LyricTransStr= LrcJson.find("TranslatedLyric").value().toString();
            LyricStr=QString::asprintf("[offset:%d]\n",LrcJson.find("LyricOffset").value().toInt())+LyricStr;
            LyricTransStr=LyricTransStr==""?"":QString::asprintf("[offset:%d]\n",LrcJson.find("LyricOffset").value().toInt())+LyricTransStr;
            int k= LRCParserInstance.ParseLRC(LyricStr);
            if(LyricTransStr!="")
            {
                k=LRCParserInstance.ParseLRC(LyricTransStr,LRCParser::LyricType::Translation);
                LRCParserInstance.GenerateMixedLyric();
            }
            auto lrcD= LRCParserInstance.GetLyricData(LyricToShow);
            for(int i=0;i<lrcD.size();i++)
            {
                ui->LyricList->addItem(lrcD[i].Line);
            }
        });
        //调用API
        NetworkM->get(QNetworkRequest("https://do.jackmyth.cn/MyMusicWebside/Search.php?Type=Lyric&SearchID="+QString::asprintf("%d",MusicToPlay.LyricID)));
    }
}

void MainWindow::RemoveFromPlaylist(MusicInfomation MusicToRemove)
{
    if(MusicCollection.size()==1)
    {
        MusicCollection.removeAll(MusicToRemove);
        CurrentMusic=-1;
    }
    else
    {
        MusicCollection.removeAll(MusicToRemove);
        if(MusicToRemove==CurrentMusicInfo)
        {
            CurrentMusic-=MusicCollection.size();
        }
        else
        {
            CurrentMusic=MusicCollection.indexOf(CurrentMusicInfo);
        }
    }
    __UpdatePlaylist();
}

void MainWindow::__UpdatePlaylist()  //更新播放列表
{
    ui->PlayList->clear();
    if(MusicCollection.size()<=0)
        return;
    //int i=CurrentMusic<0?CurrentMusic+MusicCollection.size():CurrentMusic;
    //int max=i;
    for(int i=0;i<MusicCollection.size();i++)
    {
        ui->PlayList->addItem((MusicCollection[i]==CurrentMusicInfo?QString(">"):QString())+MusicCollection[i].Name);
    }
//    do
//    {
//        auto item =new QListWidgetItem(MusicCollection[i].Name);
//        ui->PlayList->addItem(item);
//        item->setData(Qt::UserRole,QVariant::fromValue<MusicInfomation>(MusicCollection[i]));
//        //item->widget()->setUserData(0,new MusicInfomation(MusicCollection[i]));
//        i=(i+1)%MusicCollection.size();
//    }while(i!=max);
}

MusicInfomation MainWindow::GetCurrentMusic()  //获得当前音乐
{
    return CurrentMusicInfo;
}

QVector<MusicInfomation> MainWindow::GetMusicInfolist()  //获得播放列表
{
    return MusicCollection;
}

void MainWindow::on_SearchWindowButton_clicked() //打开搜索窗口
{
    if(SearchWindow::MyInstance)
        SearchWindow::MyInstance->raise();
    else
        (new SearchWindow())->show();
}

void MainWindow::on_ProgressSlider_valueChanged(int value) //processBar值改变事件
{
    //设置Label
    ui->TimeLabel->setText(QString::asprintf("%02d:%02d / %02d:%02d",
                                             MusicPlayerCore->position()/60000,MusicPlayerCore->position()%60000/1000,
                                             MusicPlayerCore->duration()/60000,MusicPlayerCore->duration()%60000/1000));
    if(!SliderPressing)
        ui->ProgressSlider->setSliderPosition(value);
}

void MainWindow::on_ProgressSlider_sliderPressed()
{
    SliderPressing=true;
}

void MainWindow::on_ProgressSlider_sliderReleased()  //放开滑块
{
    //设置音乐位置
    MusicPlayerCore->setPosition((float)ui->ProgressSlider->sliderPosition()/ui->ProgressSlider->maximum()*MusicPlayerCore->duration());
    SliderPressing=false;
}

void MainWindow::on_PlayButton_clicked()  //播放按钮
{
    //QMessageBox::information(NULL,"a",MusicPlayerCore->isSeekable()?"true":"false",QMessageBox::Ok);
    if(MusicPlayerCore->state()==QMediaPlayer::PlayingState)
        MusicPlayerCore->pause();
    else
        MusicPlayerCore->play();
}

void MainWindow::on_PreButton_clicked()  //前一个
{
    if(QApplication::mouseButtons()==2)
        MusicPlayerCore->setPlaybackRate(MusicPlayerCore->playbackRate()-0.1);
    else if(QApplication::mouseButtons()==4)
        MusicPlayerCore->setPlaybackRate(1);
    else
        PlayPre();
}

void MainWindow::on_NextButton_clicked()  //后一个
{
    if(QApplication::mouseButtons()==2)
        MusicPlayerCore->setPlaybackRate(MusicPlayerCore->playbackRate()+0.1);
    else if(QApplication::mouseButtons()==4)
        MusicPlayerCore->setPlaybackRate(1);
    else
        PlayNext();
}

void MainWindow::on_Share_Button_clicked() //显示分享菜单
{

    auto menu= UtitlityTools::ConstructShareMenu(CurrentMusicInfo); //构造分享菜单
    menu->exec(QCursor::pos());
    delete menu;  //销毁menu
}

void MainWindow::on_CommentWindowButton_clicked()   //显示评论窗口
{
    if(CommentWindow::MyInstance)
    {
        CommentWindow::MyInstance->show();
        CommentWindow::MyInstance->raise();
    }
    else
    {
        new CommentWindow();
        CommentWindow::MyInstance->move(pos().x()+this->width()+2,pos().y());
        CommentWindow::MyInstance->show();
    }
}

void MainWindow::on_SettingWindowButton_clicked() //显示设置窗口
{
    (new SettingDialog)->exec();
}

void MainWindow::on_MusicAvater_customContextMenuRequested(const QPoint &pos)
{
    if(CurrentMusicInfo.ID==0)
        return;
    QMenu* menu= UtitlityTools::ConstructPicViewMenu(
                GetPictureFromURL::SearchCache(CurrentMusicInfo.Album.PicURL));
    menu->exec(QCursor::pos());
    delete menu;
}

void MainWindow::on_Download_Button_customContextMenuRequested(const QPoint &pos)
{
    if(CurrentMusicInfo.ID==0)
        return;
    auto menu = UtitlityTools::ConstructDownloadMenu(CurrentMusicInfo);
    menu->exec(QCursor::pos());
    delete menu;
}

DownloadManager* MainWindow::GetDownloadManager()
{
    return DownloadManagerInstance;
}

void MainWindow::BeginCache(MusicInfomation MusicToCache)
{
    QNetworkReply* MusicReply = NetworkMusic->get(QNetworkRequest(MusicToCache.URL));
    connect(MusicReply,&QNetworkReply::finished,this,[=]()
    {
        if(MusicReply->error())
        {
            qDebug()<< QString("Music Cache Error:") + MusicReply->errorString();
            return;
        }
        QString Md5Hash=QCryptographicHash::hash(
                    MusicToCache.URL.toLocal8Bit(),
                    QCryptographicHash::Md5).toHex();
        qDebug()<<GlobalSetting::CacheDir +Md5Hash;
        QFile Local(GlobalSetting::CacheDir +Md5Hash);
        Local.open(QIODevice::WriteOnly);
        Local.write(MusicReply->readAll());
        Local.close();
        MusicReply->deleteLater();
    });
}

void MainWindow::on_Download_Button_clicked()
{
    on_Download_Button_customContextMenuRequested(QPoint(0,0));
}

void MainWindow::on_PlayList_customContextMenuRequested(const QPoint &pos)
{
    if(ui->PlayList->currentRow()<0)
        return;
    ui->PlayList->setCurrentRow(ui->PlayList->currentRow());
    QMenu menu;
    menu.addAction(u8"立即播放",[=]()
    {
       PlayMusic(MusicCollection[ui->PlayList->currentRow()],true);
    });
    menu.addAction(u8"下一个播放",[=]()
    {
       PlayMusicNext(MusicCollection[ui->PlayList->currentRow()]);
    });
    menu.addSeparator();
    menu.addAction(u8"移除",[=]()
    {
        RemoveFromPlaylist(MusicCollection[ui->PlayList->currentRow()]);
    });
    menu.addSeparator();
    auto shareMenu=UtitlityTools::ConstructShareMenu(MusicCollection[ui->PlayList->currentRow()]);
    //shareMenu->setParent(menu);
    menu.addMenu(shareMenu);
    menu.exec(QCursor::pos());
}
