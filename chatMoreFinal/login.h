#ifndef WELCOME_H
#define WELCOME_H

#include <QDialog>

namespace Ui {
class Login;
}

class Login : public QDialog
{
    Q_OBJECT

public:
    explicit Login(QWidget *parent = 0);
    void resizeEvent(QResizeEvent* e);
    QString gettime();
    ~Login();
    QString getUserName();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::Login *ui;

};

#endif // WELCOME_H
