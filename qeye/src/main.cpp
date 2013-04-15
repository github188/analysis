#include <QTextCodec>
#include <QCleanlooksStyle>
#include "ui_main_window.h"


#define EXTENDS     public
#define PUBLIC      public:
#define PROTECTED   protected:
#define PRIVATE     private:


// ***** QeyeWindow {{
class QeyeWindow : EXTENDS QMainWindow
{
    PUBLIC explicit QeyeWindow(void);
    PUBLIC ~QeyeWindow(void);

    PRIVATE Ui::MainWindow m_ui;
};

QeyeWindow::QeyeWindow(void)
{
    m_ui.setupUi(this);
    show();
}

QeyeWindow::~QeyeWindow(void)
{
}
// }} QeyeWindow *****


int main(int argc, char *argv[])
{
    QStyle *p_style = NULL;
    QApplication app(argc, argv);
    QeyeWindow eye_window;

    // qt初始化
    p_style = new QCleanlooksStyle();
    if (NULL == p_style) {
        return -1;
    }

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    app.setStyle(p_style);

    return app.exec();
}
