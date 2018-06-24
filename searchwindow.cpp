#include "searchwindow.h"
#include "ui_searchwindow.h"
#include "mystruct.h"
#include "mainwindow.h"
#include "globalsetting.h"
#include "utilitytools.h"
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopServices>
#include <QtNetwork/QtNetwork>

SearchWindow* SearchWindow::MyInstance=NULL;
SearchWindow::SearchWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SearchWindow)
{
    ui->setupUi(this);
    MyInstance=this;
    setAttribute(Qt::WA_DeleteOnClose);
    NetworkM=new QNetworkAccessManager();
    ui->progressBar->hide();
    ui->SearchResult_Music->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->SearchResult_Music->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->SearchResult_Music->setColumnWidth(0,300);//歌名
    ui->SearchResult_Music->setHorizontalHeaderLabels(QStringList({u8"歌曲",u8"艺术家",u8"专辑"}));
    ui->SearchResult_Music->setColumnWidth(1,195);//歌手名
    ui->SearchResult_Music->setColumnWidth(2,195);//专辑名
    //ui->SearchResult_Music->verticalHeader()->hide();
    ui->SearchResult_Music->setWordWrap(false);
    ui->SearchResult_Music->setShowGrid(false);
    ui->SearchResult_Music->setSortingEnabled(false);
    ui->SearchResult_Music->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->SearchResult_Music->setSelectionMode(QAbstractItemView::SingleSelection);
    //ui->SearchResult_Music->horizontalHeader()->setHighlightSections(true);
}

SearchWindow::~SearchWindow()
{
    delete ui;
    delete NetworkM;
    MyInstance=NULL;
}

void SearchWindow::on_Search_lineEdit_editingFinished()
{
    if(ui->Search_lineEdit->hasFocus())
    {
        BeginSearch(0);
    }
}

void SearchWindow::on_pushButton_clicked()
{
    BeginSearch(0);
}

void SearchWindow::BeginSearch(int Page)
{
    ui->progressBar->show();
    switch(ui->SearchResult_tabWidget->currentIndex())
    {
    case 0:
    {
        SearchPage=Page;
        QString SearchStr = ui->Search_lineEdit->text();
        QNetworkReply* SearchR= NetworkM->get(QNetworkRequest("https://do.jackmyth.cn/MyMusicWebside/Search.php?Search="+SearchStr+"&Type=Music"));
        connect(SearchR,&QNetworkReply::finished,this,[=]()
        {
            SearchR->deleteLater();
            ui->progressBar->hide();
            if(SearchR->error())
            {
                QMessageBox::information(nullptr,"Error",SearchR->errorString(),QMessageBox::Ok);
                return;
            }
            auto JsonRawDataArray=SearchR->readAll();
            QJsonParseError JsonError;
            QJsonDocument JsonDocument= QJsonDocument::fromJson(JsonRawDataArray,&JsonError);
            if(JsonError.error != QJsonParseError::NoError)
            {
                QMessageBox::information(nullptr,"Error",JsonError.errorString(),QMessageBox::Ok);
                return;
            }
            LastSearchResult_Music.clear();
            ui->SearchResult_Music->clearContents();
            ui->SearchResult_Music->setRowCount(0);
            QJsonArray MusicArray=JsonDocument.array();
            ui->SearchResult_Music->setRowCount(MusicArray.size());
            for(int i=0;i<MusicArray.size();i++)
            {
                MusicInfomation MI;
                QJsonObject CoreObj;
                CoreObj=MusicArray[i].toObject();
                MI.ID = CoreObj.find("MusicID").value().toInt();
                MI.Name= CoreObj.find("MusicName").value().toString();
                MI.ArtistsName=CoreObj.find("MusicianName").value().toString("Unknow Musician");
                MI.Album.ID=CoreObj.find("AlbumID").value().toInt();
                MI.Album.Name =CoreObj.find("AlbumName").value().toString("Unknow Album");
                MI.Album.PicURL=CoreObj.find("AlbumPicURL").value().toString();
                MI.URL=CoreObj.find("DownloadLink").value().toString();
                MI.LyricID=CoreObj.find("LyricID").value().toInt();
                LastSearchResult_Music.push_back(MI);
                ui->SearchResult_Music->setItem(i,0,new QTableWidgetItem(MI.Name));//歌曲名
                ui->SearchResult_Music->setItem(i,1,new QTableWidgetItem(MI.ArtistsName));//艺术家
                ui->SearchResult_Music->setItem(i,2,new QTableWidgetItem(MI.Album.Name));//专辑
            }
        });
    }
        break;
    case 1:case 2:case 3:case 4:
        ui->progressBar->hide();
    }
}

void SearchWindow::on_SearchResult_Music_cellDoubleClicked(int row, int column)
{
    (void)column;
    MainWindow::MyInstance->PlayMusic(LastSearchResult_Music[row]);
}


void SearchWindow::on_SearchPre_Button_clicked()
{
    BeginSearch(SearchPage-1);
}

void SearchWindow::on_SearchNext_Button_clicked()
{
    BeginSearch(SearchPage+1);
}

void SearchWindow::on_SearchResult_tabWidget_currentChanged(int index)
{
    switch(index)
    {
    case 0:
        ui->Search_lineEdit->setPlaceholderText(u8"搜索 音乐 专辑 艺术家");
        break;
    case 1:
        ui->Search_lineEdit->setPlaceholderText(u8"搜索 艺术家");
        break;
    case 2:
        ui->Search_lineEdit->setPlaceholderText(u8"搜索 专辑");
        break;
    case 3:
        ui->Search_lineEdit->setPlaceholderText(u8"搜索 歌词");
        break;
    case 4:
        ui->Search_lineEdit->setPlaceholderText(u8"搜索 电台");
        break;
    }
}

void SearchWindow::on_SearchResult_Music_customContextMenuRequested(const QPoint &pos)
{
    (void)pos;
    QMenu menu;
    int curRow=ui->SearchResult_Music->currentRow();
    switch(ui->SearchResult_Music->currentColumn())
    {
    case 0:
        menu.addAction(u8"立即播放",[=]()
        {
            MainWindow::MyInstance->PlayMusic(LastSearchResult_Music[curRow],true);
        });
        menu.addAction(u8"下一首播放",[=]()
        {
            MainWindow::MyInstance->PlayMusicNext(LastSearchResult_Music[curRow]);
        });
        break;
    case 1:
    case 2:
    case 3:
    case 4:
        break;
    }
    menu.addSeparator();
    menu.addMenu(UtitlityTools::ConstructDownloadMenu(LastSearchResult_Music[curRow]));
    menu.addMenu(UtitlityTools::ConstructShareMenu(LastSearchResult_Music[curRow]));
    if(QApplication::arguments().contains("-DevMode",Qt::CaseInsensitive))
        menu.addAction(QString::asprintf("%d",LastSearchResult_Music[curRow].ID));
    menu.exec(QCursor::pos());
}
