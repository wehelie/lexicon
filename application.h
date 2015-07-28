#ifndef __LEXICON_H__
#define __LEXICON_H__
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QDebug>
#include <QSettings>
#include <QFontDatabase>
#include <QWidget>
#include <QLineEdit>
#include <QRegularExpression>
#include <QDateTime>
#include <QSysInfo>
#include <iostream>
#include "QsLog.h"
/**
 *
 * This is mainly here because we need store some error messages,
 * particularly those from the XSLT processor. Having a class
 * and a pointer to the class in Lexicon class allows me declare
 * functions as 'const' that really are except for updating error messages
 */

class ApplicationErrors {
 public:
  QString takeLast();
  void setPath(const QString &);
  QString getPath() const;
  void setFile(const QString &);
  QString getFile() const;
  void add(const QString &);
 private:
  QStringList m_error;
  QString m_filePath;
  QString m_file;
};
class Lexicon : public QApplication {
  Q_OBJECT;
public:
  Lexicon(int & argc, char ** argv);
  ~Lexicon();
  enum Status { Ok, ResourceDirError, NoThemeDirectory, ThemeNotFound , SettingsNotFound};
  enum Resource { Stylesheet, Image,XSLT,Keyboard,Map,Splash,ThemeRoot};
  enum Stylesheet { Application , Entry, Print , Node };
  ApplicationErrors * m_errors;
  QString getConfig() const;
  void setOptions(const QMap<QString,QString> &);
  QMap<QString,QString> getOptions() const;
  QSettings * getSettings();
  QString settingsFileName(const QString & theme = QString());
  QString editorSettingsFileName(const QString & theme = QString());
  QString currentTheme() const { return m_currentTheme;}
  bool isOk() const;
  int setTheme(const QString &);
  void scanForFonts(const QDir &);
  QStringList getThemes() const;
  QStringList getKeyboards(int type = 0) ;
  QString     getDefaultKeyboard();
  bool        setDefaultKeyboard(const QString &);
  QString getStylesheetFilePath(int type) const;
  QString copyToTemp(const QString &);
  //  QString getResourcePath(int type);
  QString getResourceFilePath(int type,const QString & = QString()) const ;
  QString spanArabic(const QString &,const QString & which = QString());
  QString scanAndSpan(const QString &,const QString & css = QString("arabic"));
  QString scanAndStyle(const QString &,const QString & css = QString());
  QDir themeDirectory() { return m_settingsDir; }
  void setCursorPosition(QWidget *,int start=0);
  QString imageDirectory();
  QString errorPath() const;
  QString errorFile() const;
  void startLogging();
  QString takeLastError() const;
  QMap<QString,int> getUsedFont(const QString & selector = QString() , bool invertMatch = false)  ;
  QStringList setArabicFont(const QString & family = QString());
  QStringList changeFontInSettings(const QString & selector,bool match,const QString & family,int size = -1);
  QStringList changeFontInStylesheet(const QString & fileName,const QString & selector,bool matching,const QString & family,int size = -1);
  QStringList getFontInSettings(const QString & selector,bool invertMatch = false) const;
  QStringList getFontInStylesheet(const QString & fileName,const QString & selector,bool invertMatch = false) const;
  void adjustHeight(QWidget *);
  QString getSelectorCss(const QString & selector) const ;
  QFont fontFromCss(const QString & css);
  void setEditFont(QWidget *,const QString & selector,const QString & sample = QString(),int margin = 4);

  public slots:
    void onFocusChange(QWidget *,QWidget *);
 private:
    QStringList getFilteredCss(const QString & fileName) const;
    QString setCssFont(const QString & css,const QString & family,int fontSize) const;
    //    QStringList m_errors;
    //    QString m_errorFilePath;
    //    QString m_errorFile;
    QString m_configFile;
    QString m_showFirst;
    QString m_themeDirectory;
    QString m_currentTheme;
    QDir m_settingsDir;
    int m_status;
    QMap<QString,QString> m_options;
};
#endif
