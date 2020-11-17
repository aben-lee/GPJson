#ifndef TEST_H
#define TEST_H

#include <QWidget>
#include "GPJson.hpp"

namespace Ui {
class Test;
}

class Test : public QWidget
{
    Q_OBJECT

public:
    explicit Test(QWidget *parent = nullptr);
    ~Test();

private slots:
    void on_lineEdit_editingFinished();

    void on_checkBox_stateChanged(int arg1);

    void on_pushButtonReload_clicked();

    void on_pushButtonOpen_clicked();

    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    Ui::Test *ui;
    GPJson::FeatureCollect *m_features;
    QChar m_separator;
    QString m_currentFile;
    bool m_header;
    QString m_path;
};

#endif // TEST_H
