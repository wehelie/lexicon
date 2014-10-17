#include "diacriticsoptions.h"
#include "definedsettings.h"
#include "QsLog.h"
#ifndef STANDALONE
#include "application.h"
#endif
DiacriticsOptions::DiacriticsOptions(QWidget * parent) : OptionsWidget(parent) {
#ifdef STANDALONE
  m_settings = new QSettings("default.ini",QSettings::IniFormat);
  m_settings->setIniCodec("UTF-8");
#else
  m_settings = (qobject_cast<Lexicon *>(qApp))->getSettings();
#endif
  m_section = "Diacritics";

  m_chars.insert(0x0621,"Hamza");
  m_chars.insert(0x0622,"Alef with madda above");
  m_chars.insert(0x0623,"Alef with hamza above");
  m_chars.insert(0x0624,"Waw with hamza above");
  m_chars.insert(0x0625,"Alef with hamza below");
  m_chars.insert(0x0626,"Yeh with hamza above");
  m_chars.insert(0x0627,"Alef");
  m_chars.insert(0x0628,"Beh");
  m_chars.insert(0x0629,"Teh marbuta");
  m_chars.insert(0x062a,"Teh");
  m_chars.insert(0x062b,"Theh");
  m_chars.insert(0x062c,"Jeem");
  m_chars.insert(0x062d,"Hah");
  m_chars.insert(0x062e,"Khah");
  m_chars.insert(0x062f,"Dal");
  m_chars.insert(0x0630,"Thal");
  m_chars.insert(0x0631,"Reh");
  m_chars.insert(0x0632,"Zain");
  m_chars.insert(0x0633,"Seen");
  m_chars.insert(0x0634,"Sheen");
  m_chars.insert(0x0635,"Sad");
  m_chars.insert(0x0636,"Dad");
  m_chars.insert(0x0637,"Tah");
  m_chars.insert(0x0638,"Zah");
  m_chars.insert(0x0639,"Ain");
  m_chars.insert(0x063a,"Ghain");
  m_chars.insert(0x0641,"Feh");
  m_chars.insert(0x0642,"Qaf");
  m_chars.insert(0x0643,"Kaf");
  m_chars.insert(0x0644,"Lam");
  m_chars.insert(0x0645,"Meem");
  m_chars.insert(0x0646,"Noon");
  m_chars.insert(0x0647,"Heh");
  m_chars.insert(0x0648,"Waw");
  m_chars.insert(0x0649,"Alef maksura");
  m_chars.insert(0x064a,"Yeh");
  m_chars.insert(0x064b,"Fathatan");
  m_chars.insert(0x064c,"Dammatan");
  m_chars.insert(0x064d,"Kasratan");
  m_chars.insert(0x064e,"Fatha");
  m_chars.insert(0x064f,"Damma");
  m_chars.insert(0x0650,"Kasra");
  m_chars.insert(0x0651,"Shadda");
  m_chars.insert(0x0652,"Sukun");
  m_chars.insert(0x0654,"Hamza above");
  m_chars.insert(0x0670,"Superscript alef");
  m_chars.insert(0x0671,"Alef wasla");
  m_chars.insert(0x067e,"Peh");
  m_chars.insert(0x0686,"Tcheh");
  m_chars.insert(0x0698,"Jeh");
  m_chars.insert(0x06a4,"Veh");
  m_chars.insert(0x06af,"Gaf");

  QVBoxLayout * vlayout = new QVBoxLayout;
  QGridLayout * gridlayout = new QGridLayout;;
  //  gridlayout->setSpacing(0);
  QMapIterator<int,QString> i(m_chars);
  int row=0;
  int col=0;

  while(i.hasNext()) {
    i.next();
    QCheckBox * box = new QCheckBox(i.value());
    box->setObjectName("charname");
    box->setProperty("codepoint", i.key());
    connect(box,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
    gridlayout->addWidget(box,row,col);
    col++;
    if (col == 4) {
      row++;
      col = 0;
    }
  }
  QStringList html;

  html << tr("The characters presented here form a complete list of those characters in the Unicode Arabic character set (Range 0600-06ff) that occur in the lexicon.");
  html << "<br/>";
  html << tr("Any character whose checkbox is ticked will be invisible to the search routine when the 'Ignore Diacritics' option is also checked.");

  QTextEdit * info = new QTextEdit;
  info->setHtml(html.join(""));
  info->setReadOnly(true);
  vlayout->addLayout(gridlayout);
  vlayout->addWidget(info);
  setLayout(vlayout);
  readSettings();
  setupConnections();
}
void DiacriticsOptions::readSettings() {
  QString hex;
  bool ok;
  QLOG_DEBUG() << Q_FUNC_INFO;

  QList<QCheckBox *> boxes = this->findChildren<QCheckBox *>();
  QStringList points;
  m_settings->beginGroup(m_section);
  QStringList keys = m_settings->childKeys();
  for(int i=0;i < keys.size();i++) {
    if (keys[i].startsWith("Char")) {
      hex = m_settings->value(keys[i],QString()).toString();
      int x = hex.toInt(&ok,16);
      points << hex;
      for(int j=0;j < boxes.size();j++) {
        if (boxes[j]->property("codepoint") == x) {
          boxes[j]->setChecked(true);
          j = boxes.size();
        }
      }
    }
  }
  m_settings->endGroup();

  /// Test code
  QString rx = QString("[\\x%1]*").arg(points.join("\\x"));
  QRegExp p(rx);
  QRegExp rxclass("[\\x064b\\x0671\\x064c\\x064d\\x064e\\x064f\\x0650\\x0651\\x0652\\x0670]*");
  qDebug() << p.pattern();
  qDebug() << rxclass.pattern();
  qDebug() << "Rx match" << (p.pattern() == rxclass.pattern());
}
void DiacriticsOptions::writeSettings() {
  QList<int> codepoints;
  QList<QCheckBox *> boxes = this->findChildren<QCheckBox *>();
  for(int j=0;j < boxes.size();j++) {
    if (boxes[j]->isChecked()) {
      codepoints << boxes[j]->property("codepoint").toInt();
    }
  }

  m_settings->beginGroup(m_section);
  m_settings->remove("");
  QString hexstr;
  QTextStream str(&hexstr);
  str.setFieldWidth(4);
  str.setPadChar('0');
  for(int i=0;i < codepoints.size();i++) {
      str << hex << codepoints[i];
      m_settings->setValue(QString("Char%1").arg(i+1),hexstr);
      str.flush();
      hexstr.clear();
  }
  m_settings->endGroup();
}
bool DiacriticsOptions::isModified()  {
  bool ok;
  QString s;
  m_dirty = false;
  QList<int> saved_cp;
  QList<int> selected_cp;
  QString hex;
  qDebug() << Q_FUNC_INFO;
  m_settings->beginGroup(m_section);
  QStringList keys = m_settings->childKeys();
  for(int i=0;i < keys.size();i++) {
    if (keys[i].startsWith("Char")) {
      hex = m_settings->value(keys[i],QString()).toString();
      int x = hex.toInt(&ok,16);
      saved_cp << x;
    }
  }
  m_settings->endGroup();
  QList<QCheckBox *> boxes = this->findChildren<QCheckBox *>();
  for(int j=0;j < boxes.size();j++) {
    if (boxes[j]->isChecked()) {
      selected_cp << boxes[j]->property("codepoint").toInt();
    }
  }
  if (saved_cp.size() != selected_cp.size()) {
    m_dirty = true;
    return true;
  }
  for(int i=0;i < selected_cp.size();i++) {
    if (! saved_cp.contains(selected_cp[i])) {
      m_dirty = true;
      return true;
    }
  }
  return m_dirty;
}