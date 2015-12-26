#ifndef _USERTABLEWIDGET_H_
#define _USERTABLEWIDGET_H_
#include <QApplication>
#include <QMainWindow>
#include <QFile>
#include <ostream>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QSettings>
#include <QLabel>
/*************************************************************************/
/* QStringList cols;                                                     */
/* cols << "One" << "Two" << "Three";                                    */
/* ColumnarTableWidget * t = new ColumnarTableWidget(cols);                                  */
/* QSettings settings("settings.ini",QSettings::IniFormat);              */
/* settings.setIniCodec("UTF-8");                                        */
/* settings.beginGroup("Test");                                          */
/* t->setKey(ColumnarTableWidget::STATE,"mystate");                                */
/* t->readConfiguration(settings);                                       */
/* ...carry on, load data etc etc                                        */
/*************************************************************************/
class ColumnarTableWidget : public QTableWidget {
  Q_OBJECT

 public:
  ColumnarTableWidget(const QStringList & headers,QWidget * parent = 0);
  ~ColumnarTableWidget();
  void setKey(int,const QString &);
  void setSaveConfiguration(bool v);
  void setDefaultWidth(int);
  void showEmpty(const QString &);
  enum { DEFAULT_WIDTH, COLUMN_WIDTHS,STATE };
  virtual void readConfiguration(QSettings &);
  virtual void writeConfiguration();
  virtual QLabel * createLabel(const QString &,const QString & style = QString()) const;
 private slots:
    void onColumnDialog(int);
 private:
    bool startsWithArabic(const QString &) const;
    QStringList m_colHeadings;
    QString m_defaultWidthKey;
    QString m_columnWidthsKey;
    QString m_stateKey;
    QString m_emptyText;
    QSettings * m_settings;
    int m_defaultWidth;
    bool m_saveConfig;
};
#endif
