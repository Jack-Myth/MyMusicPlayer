#ifndef REGISTERANDLOGIN_H
#define REGISTERANDLOGIN_H

#include <QMainWindow>

namespace Ui {
class RegisterAndLogin;
}

class RegisterAndLogin : public QMainWindow
{
    Q_OBJECT

public:
    static void Show();
    explicit RegisterAndLogin(QWidget *parent = 0);
    ~RegisterAndLogin();
    static QString GetGlobalToken();
    static QString GetUserID();
private slots:
    void on_radioButton_2_clicked();

    void on_radioButton_clicked();

    void on_pushButton_2_clicked();

private:
    static RegisterAndLogin* MyInstance;
    static QString GlobalToken;
    static QString UserID;
    Ui::RegisterAndLogin *ui;
};

#endif // REGISTERANDLOGIN_H
