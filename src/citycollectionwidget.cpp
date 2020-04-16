#include "citycollectionwidget.h"
#include "ui_citycollectionwidget.h"
#include "cityaddwidget.h"
#include "citycollectionitem.h"

#include <QFile>
#include <QApplication>
#include <QStringList>

CityCollectionWidget::CityCollectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::citycollectionwidget)
{
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);//设置窗口背景透明
    QPainterPath path;
    auto rect = this->rect();
    rect.adjust(1, 1, -1, -1);
    path.addRoundedRect(rect, 6, 6);
    setProperty("blurRegion", QRegion(path.toFillPolygon().toPolygon()));
    this->setStyleSheet("QWidget{border:none;border-radius:6px;}");
    this->setWindowIcon(QIcon::fromTheme("indicator-china-weather", QIcon(":/res/control_icons/indicator-china-weather.png")) );

    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width()-this->width())/2, (availableGeometry.height()-this->height())/2);

    ui->backwidget->setStyleSheet("QWidget{border:1px solid rgba(255,255,255,0.05);border-radius:6px;background:rgba(255,255,255,1);}");

    ui->lbLeftUpIcon->setStyleSheet("QLabel{border:none;background:transparent;background-image:url(':/res/control_icons/logo.png');}");

    ui->lbLeftUpTitle->setStyleSheet("QLabel{border:none;background:transparent;font-size:14px;font-weight:400;color:rgba(68,68,68,1);}");
    ui->lbLeftUpTitle->setText("麒麟天气");

    ui->btnCancel->setStyleSheet("QPushButton{border:0px;background:transparent;background-image:url(:/res/control_icons/close_black.png);}");

    ui->lbCityCurrent->setStyleSheet("QLabel{border:none;background:transparent;font-size:18px;font-weight:400;color:rgba(68,68,68,1);}");
    ui->lbCityCurrent->setText("当前城市");

    ui->lbCityCollect->setStyleSheet("QLabel{border:none;background:transparent;font-size:18px;font-weight:400;color:rgba(68,68,68,1);}");
    ui->lbCityCollect->setText("收藏城市");

    ui->lbCityCount->setStyleSheet("QLabel{border:none;background:transparent;font-size:12px;font-weight:400;color:rgba(68,68,68,1);}");
    ui->lbCityCount->setText("1/8");

    m_cityaddition = new CityAddition(this);
    m_cityaddition->move(0, 0);
    m_cityaddition->hide();
    connect(m_cityaddition, SIGNAL(hideCityAddWiget()), this, SLOT(onHideCityAddWiget()) );
    connect(m_cityaddition, SIGNAL(requestAddNewCity(QString)), this, SLOT(onRequestAddNewCity(QString)) );

    setCurrentCity(); //显示当前城市
    setCollectCity(); //显示收藏城市
}

CityCollectionWidget::~CityCollectionWidget()
{
    delete ui;
}

void CityCollectionWidget::setCurrentCity()
{
    citycollectionitem *m_currentcity = new citycollectionitem(ui->backwidget);
    m_currentcity->move(35, 81);
    m_currentcity->setItemWidgetState(true, true);
    m_currentcity->show();

    QString strCurrCity = readCollectedCity();
    QStringList listCityId = strCurrCity.split(",");
    QString cityId = "";
    if (listCityId.size() > 1){
        cityId = listCityId.at(0);
    } else {
        cityId = "101010100"; //使用北京的ID
    }
    m_currentcity->setCurrentWeather(cityId);
    connect(m_currentcity, SIGNAL(requestDeleteCity(QString)), this, SLOT(onRequestDeleteCity(QString)) );
}

void CityCollectionWidget::setCollectCity()
{
    QString strSavedCity = readCollectedCity();

    QStringList listSavedCityId = strSavedCity.split(",");
    int cityNumber = listSavedCityId.size() - 1; //减掉一个含空字符的项

    int row = 0; //当前行
    int column = 0; //当前列
    if (cityNumber > 1){
        //先不考虑当前城市这个item
        for (int i=1;i<cityNumber;i++){
            showCollectCity(35 + column*170, 242 + row*100, true, listSavedCityId.at(i));
            column += 1;
            if (column == 3){
                column = 0;
                row += 1;
            }
        }
    }
    showCollectCity(35 + column*170, 242 + row*100, false, "");
}

void CityCollectionWidget::showCollectCity(int x, int y, bool isShowNormal, QString cityId)
{
    citycollectionitem *m_currentcity = new citycollectionitem(ui->backwidget);
    m_currentcity->move(x, y); //m_currentcity->move(35 + j*170, 242 + i*100);
    m_currentcity->setItemWidgetState(isShowNormal, false);
    m_currentcity->show();
    m_currentcity->setCurrentWeather(cityId);
    connect(m_currentcity, SIGNAL(showCityAddWiget()), this, SLOT(onShowCityAddWiget()) );
    connect(m_currentcity, SIGNAL(requestDeleteCity(QString)), this, SLOT(onRequestDeleteCity(QString)) );
    connect(m_currentcity, SIGNAL(changeCurrentCity(QString)), this, SLOT(onChangeCurrentCity(QString)) );
}

void CityCollectionWidget::onRequestAddNewCity(QString cityId)
{
    m_cityaddition->hide(); //隐藏 搜索及添加城市窗口

    //第一步 移动控件
    QList<citycollectionitem *> cityItemList = ui->backwidget->findChildren<citycollectionitem *>();

    int itemNum = cityItemList.size();

    citycollectionitem *lastCityItem = cityItemList.at(itemNum-1); //删除最后一项
    delete lastCityItem;
    itemNum -= 1;

    if (itemNum <= 1) {
        showCollectCity(35, 242, true, cityId); //添加新增加的收藏城市
        showCollectCity(35, 242 + 100, false, ""); //添加最后一项
    }

    if (itemNum > 1 && itemNum < 9) {
        int row(0), column(0);
        for (int i = 1;i < itemNum; i ++) {
            column += 1;
            if (column == 3) {
                column = 0;
                row += 1;
            }
        }
        showCollectCity(35 + column*170, 242 + row*100, true, cityId); //添加新增加的收藏城市

        column += 1;
        if (column == 3) {
            column = 0;
            row += 1;
        }
        showCollectCity(35 + column*170, 242 + row*100, false, ""); //添加最后一项
    }

    if (itemNum >= 9) {
        QList<citycollectionitem *> m_cityItemList = ui->backwidget->findChildren<citycollectionitem *>();
        citycollectionitem *m_lastCityItem = m_cityItemList.at(m_cityItemList.size() - 1); //删除最后一项
        delete m_lastCityItem;
        showCollectCity(35 + 1*170, 242 + 2*100, true, cityId); //添加新增加的收藏城市
        showCollectCity(35 + 2*170, 242 + 2*100, false, ""); //添加最后一项
    }

    //第二步 将新增城市写入列表
    QString strSavedCity = readCollectedCity();
    QStringList listSavedCityId = strSavedCity.split(",");
    if (listSavedCityId.size() == 10){ //包含最后一项为空字符串的项
        listSavedCityId.replace(8, cityId); //收藏城市已经有8个，替换最后一个收藏城市
    }else {
        listSavedCityId.append(cityId); //若收藏城市未满8个,将新添加的城市加到最后
    }

    QString newStrCityId = "";
    foreach(QString str, listSavedCityId){
        if (str != ""){
            newStrCityId.append(str);
            newStrCityId.append(",");
        }
    }

    writeCollectedCity(newStrCityId);
}

void CityCollectionWidget::onRequestDeleteCity(QString cityId)
{
    qDebug()<<"debug: city id = "<<cityId;

    QString strSavedCity = readCollectedCity();
    QStringList listSavedCityId = strSavedCity.split(",");

    //若收藏窗口只有当前城市，不能删掉当前城市
    if (listSavedCityId.size() == 2) {
        return;
    }

    //删掉对应的城市
    QList<citycollectionitem *> cityItemList = ui->backwidget->findChildren<citycollectionitem *>();
    for(int i = 0;i < cityItemList.size(); i ++){
        citycollectionitem *cityItem = cityItemList.at(i);
        if (i == 0) {
            if (cityItem->m_city_id == cityId) { //说明删除的是当前城市，以第一个收藏城市代替
                emit sendCurrentCityId(listSavedCityId.at(1)); //发信号更新主界面
                delete cityItem;
            }
        } else {
            if (cityItem->m_city_id == cityId) {
                delete cityItem;
            }
        }
    }

    //重新排列现有收藏城市
    int row(0), column(0);
    QList<citycollectionitem *> newCityItemList = ui->backwidget->findChildren<citycollectionitem *>();
    for(int i = 0;i < newCityItemList.size(); i ++){
        citycollectionitem *newCityItem = newCityItemList.at(i);
        if (i == 0) {
            newCityItem->move(35, 81); //当前城市
            newCityItem->setItemWidgetState(true, true);
        } else {
            newCityItem->move(35 + column*170, 242 + row*100); //收藏城市
            column += 1;
            if (column == 3) {
                column = 0;
                row += 1;
            }
        }
    }

    //更新城市列表
    bool isflag = listSavedCityId.removeOne(cityId);
    if (isflag) {
        qDebug()<<"delete one element from collected city list successfully";
        QString newStrCityId = "";
        foreach(QString str, listSavedCityId){
            if (str != ""){
                newStrCityId.append(str);
                newStrCityId.append(",");
            }
        }
        writeCollectedCity(newStrCityId);
    } else {
        qDebug()<<"delete one element from collected city list failed";
    }
}

void CityCollectionWidget::onChangeCurrentCity(QString cityId)
{
    emit sendCurrentCityId(cityId); //发信号更新主界面

    //交换城市item
    QList<citycollectionitem *> cityItemList = ui->backwidget->findChildren<citycollectionitem *>();
    citycollectionitem *firstCityItem = cityItemList.at(0);
    firstCityItem->setItemWidgetState(true, false);
    for(int i = 1;i < cityItemList.size(); i ++){
        citycollectionitem *cityItem = cityItemList.at(i);
        if (cityItem->m_city_id == cityId) {
            citycollectionitem *swapCityItem = cityItemList.at(i);
            swapCityItem->setItemWidgetState(true, true);
            cityItemList.replace(i, firstCityItem);
            cityItemList.replace(0, swapCityItem);
            break;
        }
    }

    //重新排列现有收藏城市
    int row(0), column(0);
    for(int i = 0;i < cityItemList.size(); i ++){
        citycollectionitem *newCityItem = cityItemList.at(i);
        if (i == 0) {
            newCityItem->move(35, 81); //当前城市
        } else {
            newCityItem->move(35 + column*170, 242 + row*100); //收藏城市
            column += 1;
            if (column == 3) {
                column = 0;
                row += 1;
            }
        }
    }

    //更新城市列表
    QString strSavedCity = readCollectedCity();
    QStringList listSavedCityId = strSavedCity.split(",");

    QString firstId = listSavedCityId.at(0);
    listSavedCityId.replace(0, cityId);

    for (int i=1;i<listSavedCityId.size();i++){
        if (cityId == listSavedCityId.at(i)){
            listSavedCityId.replace(i, firstId);
        }
    }

    QString newStrCityId = "";
    foreach(QString str, listSavedCityId){
        if (str != ""){
            newStrCityId.append(str);
            newStrCityId.append(",");
        }
    }

    writeCollectedCity(newStrCityId);
}

void CityCollectionWidget::writeCollectedCity(QString cityId)
{
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString collectPath = homePath.at(0) + "/.config/china-weather-data";

    QFile file(collectPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(cityId.toUtf8());
    file.close();
}

QString CityCollectionWidget::readCollectedCity()
{
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString collectPath = homePath.at(0) + "/.config/china-weather-data";

    QFile file(collectPath);
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray cityId = file.readAll();
    QString readCityId = (QString(cityId));
    file.close();

    return readCityId;
}

void CityCollectionWidget::onShowCityAddWiget()
{
    m_cityaddition->show();
    m_cityaddition->raise();
}

void CityCollectionWidget::onHideCityAddWiget()
{
    m_cityaddition->hide();
}

void CityCollectionWidget::mousePressEvent(QMouseEvent *event){
    if(event->button() == Qt::LeftButton){
        this->isPress = true;
        this->winPos = this->pos();
        this->dragPos = event->globalPos();
        event->accept();
    }
}

void CityCollectionWidget::mouseReleaseEvent(QMouseEvent *event){
    this->isPress = false;
}

void CityCollectionWidget::mouseMoveEvent(QMouseEvent *event){
    if(this->isPress){
        this->move(this->winPos - (this->dragPos - event->globalPos()));
        event->accept();
    }
}

void CityCollectionWidget::on_btnCancel_clicked()
{
    this->hide();
}
