#include "logoptions.h"
#include "definedsettings.h"
#include "QsLog.h"
#ifndef STANDALONE
#include "application.h"
#endif
/**
 *
 * @param theme
 * @param parent
 */
LogOptions::LogOptions(const QString & theme,QWidget * parent) : OptionsWidget(theme,parent) {
  m_section = "Logging";


  QVBoxLayout * vlayout = new QVBoxLayout;

  QFormLayout * layout = new QFormLayout;

  m_file = new QLineEdit ;
  m_maxSize = new QLineEdit ;
  m_maxSize->setValidator(new QIntValidator);
  m_level = new QSpinBox ;
  m_archive = new QSpinBox ;
  m_maxLines = new QLineEdit ;
  m_maxLines->setValidator(new QIntValidator);
  m_interval = new QSpinBox ;
  m_rotate = new QCheckBox ;
  m_warning = new QLineEdit ;
  m_error = new QLineEdit ;
  m_debug = new QLineEdit ;
  m_info = new QLineEdit ;

  layout->addRow(tr("File name"),m_file);
  layout->addRow(tr("Maximum file size (bytes)"),m_maxSize);
  layout->addRow(tr("Log level"),m_level);
  layout->addRow(tr("Number of archives"),m_archive);
  layout->addRow(tr("Viewer lines"),m_maxLines);
  layout->addRow(tr("Viewer refresh interval (secs)"),m_interval);
  layout->addRow(tr("Rotate logs"),m_rotate);
  layout->addRow(tr("Warning icon"),m_warning);
  layout->addRow(tr("Information icon"),m_info);
  layout->addRow(tr("Error icon"),m_error);
  layout->addRow(tr("Debug icon"),m_debug);
  vlayout->addLayout(layout);
  vlayout->addStretch();
  setLayout(vlayout);
  addButtons();
  readSettings();
  setupConnections();

}

void LogOptions::readSettings() {
  qDebug() << Q_FUNC_INFO << m_settingsFileName;
  QSettings settings(m_settingsFileName,QSettings::IniFormat);
  settings.setIniCodec("UTF-8");
  settings.beginGroup(m_section);

  m_file->setText(settings.value(SID_LOGGING_FILE,"log.txt").toString());
  m_maxSize->setText(settings.value(SID_LOGGING_MAXSIZE,"64000").toString());
  /// TODO check max/min
  m_level->setValue(settings.value(SID_LOGGING_LEVEL,2).toInt());
  m_archive->setValue(settings.value(SID_LOGGING_ARCHIVES,4).toInt());
  m_maxLines->setText(settings.value(SID_LOGGING_VIEWER_MAXLINES,"100").toString());
  m_interval->setValue(settings.value(SID_LOGGING_VIEWER_INTERVAL,30).toInt());
  m_rotate->setChecked(settings.value(SID_LOGGING_ROTATE,true).toBool());
  m_warning->setText(settings.value(SID_LOGGING_ICON_WARNING,"warn.png").toString());
  m_info->setText(settings.value(SID_LOGGING_ICON_INFO,"info.png").toString());
  m_error->setText(settings.value(SID_LOGGING_ICON_ERROR,"error.png").toString());
  m_debug->setText(settings.value(SID_LOGGING_ICON_DEBUG,"debug.png").toString());

  m_dirty = false;
}
void LogOptions::writeSettings(const QString & fileName) {
  QString f = m_settingsFileName;
  if (!fileName.isEmpty()) {
    f = fileName;
  }
  qDebug() << Q_FUNC_INFO << f;

  QSettings settings(f,QSettings::IniFormat);
  settings.setIniCodec("UTF-8");
  settings.beginGroup(m_section);

  m_dirty = false;
  emit(modified(false));
}
/**
 * TODO not complete
 *
 *
 * @return
 */
bool LogOptions::isModified()  {
  m_dirty = false;
  QSettings settings(m_settingsFileName,QSettings::IniFormat);
  settings.setIniCodec("UTF-8");
  settings.beginGroup(m_section);

  return m_dirty;
}
void LogOptions::onSetFont() {
  /*
  QFont f;
  f.fromString(m_font->text());

  QFontDialog * d = new QFontDialog(f);
  QList<QComboBox *> boxes = d->findChildren<QComboBox *>();
  if (boxes.size() == 1) {
    int ix = boxes[0]->findText("Arabic");
    if (ix >= 0) {
      boxes[0]->setCurrentText("Arabic");
      boxes[0]->activated(ix);
    }
  }
  d->setCurrentFont(f);
  if (d->exec() == QDialog::Accepted) {
    QFont font = d->currentFont();
    m_font->setText(font.toString());
  }
  delete d;
  */
  return;
}
void LogOptions::onSetColor() {
  /*
  QColor color;
  color.setNamedColor(m_highlightColor->text());
  QColorDialog d(color);
  if (d.exec() != QDialog::Accepted) {
    return;
  }
  int r,g,b;
  color = d.currentColor();
  color.getRgb(&r,&g,&b);
  QString str = QString("%1,%2,%3").arg(r).arg(g).arg(b);
  QSettings settings(m_settingsFileName,QSettings::IniFormat);
  settings.setIniCodec("UTF-8");
  settings.beginGroup("Colors");
  QStringList keys = settings.allKeys();
  QStringList v;
  for(int i=0;i < keys.size();i++) {
    v = settings.value(keys[i]).toStringList();
    if (v.join(",") == str) {

      m_highlightColor->setText(keys[i]);
      return;
    }
  }
  m_highlightColor->setText(d.currentColor().name());
  */
}
