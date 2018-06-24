#ifndef COMMENTWINDOW_H
#define COMMENTWINDOW_H

#include <QMainWindow>
#include "mystruct.h"
#include "utilitytools.h"
#include <qt_windows.h>

namespace Ui {
class CommentWindow;
}

class CommentWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit CommentWindow(QWidget *parent = 0);
    ~CommentWindow();
    static CommentWindow* MyInstance;
    void GetComment(MusicInfomation Music, int Page);
    QString CurrentUserName="";
    void Logined();
    QVector<int> CommentIDs;
signals:
    void WindowMoved();
protected:
    friend LRESULT CALLBACK CommentWindow_MyWndProc(HWND wnd,UINT message,WPARAM wParam,LPARAM lParam);
    WNDPROC SuperWndProc=NULL;
private slots:
    void on_NextButton_clicked();

    void on_PreButton_clicked();

    void on_RefreshButton_clicked();

    void on_LoginAndComment_clicked();

    void on_CommentList_customContextMenuRequested(const QPoint &pos);

private:
    struct SameLabelCollection
    {
        QString url;
        QVector<class QLabel*> LabelCollection;
        class GetPictureFromURL* pGetPictureFromURL;
        inline void Clear()
        {
            pGetPictureFromURL->Abandon();
            LabelCollection.clear();
        }
    };
    Ui::CommentWindow *ui;
    QVector<SameLabelCollection*> SameLabelCollectionInstance;
    int CurrentPage=0;
    class QNetworkAccessManager* NetworkM=NULL;
    void AddComment(QJsonObject JsonObject);
    void onWindowMoved();
    void SubmitComment(int MusicID,int TargetComment);
};

#endif // COMMENTWINDOW_H
