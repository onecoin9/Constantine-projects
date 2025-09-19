 #include "Ag06TrimDialog.h"
 #include <QJsonParseError>
 #include <QHeaderView>
 #include <QTableWidgetItem>
 #include <QFile>
 #include <QIODevice>
 #include <QColor>

 Ag06TrimDialog::Ag06TrimDialog(QWidget* parent)
     : QDialog(parent)
 {
     setWindowTitle("AG06 二进制 DoCustom / Trim 编辑器");
     resize(900, 700);
     buildUi();
 }

 void Ag06TrimDialog::buildUi()
 {
     auto* main = new QVBoxLayout(this);
    m_tabs = new QTabWidget(this);
    setupBinaryTab();
    setupTableTab();
     main->addWidget(m_tabs);
 }

 void Ag06TrimDialog::setupBinaryTab()
 {
     m_binaryTab = new QWidget;
     auto* v = new QVBoxLayout(m_binaryTab);

     auto* uidRow = new QHBoxLayout;
     uidRow->addWidget(new QLabel("UID(8字符):"));
     m_uidEdit = new QLineEdit;
     m_uidEdit->setPlaceholderText("例如: B8200000");
     m_sendUidBtn = new QPushButton("下发 UID (0x10)");
     uidRow->addWidget(m_uidEdit);
     uidRow->addWidget(m_sendUidBtn);
     v->addLayout(uidRow);

     v->addWidget(new QLabel("Trim 参数(JSON, 映射 xt_trim_t):"));
     m_trimJsonEdit = new QTextEdit;
     m_trimJsonEdit->setPlaceholderText("请输入包含 t1_trim_en/t1_trim_regs/t1_output_ctrl_value/t1_trim_params/delay_set 的 JSON");
     m_trimJsonEdit->setMaximumHeight(200);
     v->addWidget(m_trimJsonEdit);

     m_sendTrimJsonBtn = new QPushButton("下发 Trim JSON (0x11)");
     v->addWidget(m_sendTrimJsonBtn, 0, Qt::AlignRight);

     m_status = new QLabel("状态: 待操作");
     v->addWidget(m_status);

     connect(m_sendUidBtn, &QPushButton::clicked, this, [this]{ emit sendUidRequested(m_uidEdit->text().trimmed()); });
     connect(m_sendTrimJsonBtn, &QPushButton::clicked, this, [this]{ emit sendTrimJsonRequested(m_trimJsonEdit->toPlainText()); });

     m_tabs->addTab(m_binaryTab, "二进制");
 }

 // 旧的表单页已移除，统一用表格页编辑

void Ag06TrimDialog::setupTableTab()
{
    // 以表格作为唯一编辑界面：中文名 | 结构体字段 | 类型 | 值
    QWidget* tab = new QWidget;
    auto* v = new QVBoxLayout(tab);
    m_table = new QTableWidget(0, 5, tab);
    m_table->setHorizontalHeaderLabels({"中文名称", "结构体字段", "类型", "值", "提示/范围"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    v->addWidget(m_table);

    auto* btn = new QHBoxLayout;
    m_tableImportJsonBtn = new QPushButton("导入JSON...");
    m_tableExportJsonBtn = new QPushButton("导出JSON...");
    m_tableSendTrimBtn   = new QPushButton("下发 Trim 结构体 (0x11)");
    btn->addWidget(m_tableImportJsonBtn);
    btn->addWidget(m_tableExportJsonBtn);
    btn->addStretch();
    btn->addWidget(m_tableSendTrimBtn);
    auto* btnW = new QWidget; btnW->setLayout(btn);
    v->addWidget(btnW);

    // 状态栏
    m_tableStatus = new QLabel("就绪");
    v->addWidget(m_tableStatus);

    connect(m_tableImportJsonBtn, &QPushButton::clicked, this, [this]{
        QString path = QFileDialog::getOpenFileName(this, "打开 Trim JSON", {}, "JSON (*.json)");
        if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QJsonParseError perr{}; QJsonDocument d = QJsonDocument::fromJson(f.readAll(), &perr);
            if (perr.error == QJsonParseError::NoError && d.isObject()) loadTrimJson(d.object());
        }
    });
    connect(m_tableExportJsonBtn, &QPushButton::clicked, this, [this]{
        QJsonObject o = buildTrimJson();
        QString path = QFileDialog::getSaveFileName(this, "保存 Trim JSON", "trim.json", "JSON (*.json)");
    if (path.isEmpty()) return;
        QFile f(path);
        if (f.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) { f.write(QJsonDocument(o).toJson(QJsonDocument::Indented)); f.close(); }
    });
    connect(m_tableSendTrimBtn, &QPushButton::clicked, this, [this]{ emit sendTrimStructRequested(collectTrim()); });

    m_tabs->addTab(tab, "表格编辑");
    buildTable();
    // 编辑时实时校验
    connect(m_table, &QTableWidget::cellChanged, this, [this](int r,int c){ if (c==3) validateRow(r); });
}

void Ag06TrimDialog::buildTable()
{
    // 构建行定义：覆盖 xt_trim_t 的所有字段
    m_rowDefs.clear();
    m_rowIndex.clear();
    auto add=[&](const char* zh,const char* path,const char* type,const char* unit=nullptr,quint64 minV=0,quint64 maxV=0,const char* example=nullptr){ m_rowIndex[path]=m_rowDefs.size(); m_rowDefs.push_back({QString::fromUtf8(zh), path, type, unit?QString::fromUtf8(unit):QString(), minV, maxV, example?QString::fromUtf8(example):QString()}); };

    // t1_trim_en.value（用数值表示 5 个bit）
    add("Trim 步骤使能掩码(value)", "t1_trim_en.value", "uint32_t", "bitmask", 0, 0x1F, "示例: 0x1F/31=全开");

    // t1_trim_regs.* 四元组
    const char* regs[] = {"output_ctrl","dc_trim","ac_en","ac_trim","eoc"};
    for (auto r: regs) { add(QString("t1_trim_regs.%1.addr").arg(r).toUtf8().constData(), QString("t1_trim_regs.%1.addr").arg(r).toUtf8().constData(), "uint8_t", nullptr, 0, 255, "0~255"); }
    for (auto r: regs) { add(QString("t1_trim_regs.%1.start_bit").arg(r).toUtf8().constData(), QString("t1_trim_regs.%1.start_bit").arg(r).toUtf8().constData(), "uint8_t", nullptr, 0, 31, "0~31"); }
    for (auto r: regs) { add(QString("t1_trim_regs.%1.width_bit").arg(r).toUtf8().constData(), QString("t1_trim_regs.%1.width_bit").arg(r).toUtf8().constData(), "uint8_t", "bits", 0, 32, "0~32"); }
    for (auto r: regs) { add(QString("t1_trim_regs.%1.write_back").arg(r).toUtf8().constData(), QString("t1_trim_regs.%1.write_back").arg(r).toUtf8().constData(), "uint8_t", "bool", 0, 1, "0或1"); }

    // t1_output_ctrl_value
    add("输出控制 dc6","t1_output_ctrl_value.dc6","uint8_t", nullptr, 0, 255, "0~255");
    add("输出控制 dc5","t1_output_ctrl_value.dc5","uint8_t", nullptr, 0, 255, "0~255");
    add("输出控制 ac27","t1_output_ctrl_value.ac27","uint8_t", nullptr, 0, 255, "0~255");
    add("输出控制 ac4","t1_output_ctrl_value.ac4","uint8_t", nullptr, 0, 255, "0~255");

    // t1_trim_params
    add("工作电流 icc_min","t1_trim_params.icc_min","uint32_t", "uA", 0, 2000000000ull, "示例: 10000");
    add("工作电流 icc_max","t1_trim_params.icc_max","uint32_t", "uA", 0, 2000000000ull, "示例: 50000");
    add("直流最小 dc_basic_min","t1_trim_params.dc_basic_min","uint16_t", "mV", 0, 65535, "0~65535");
    add("直流最大 dc_basic_max","t1_trim_params.dc_basic_max","uint16_t", "mV", 0, 65535, "0~65535");
    add("直流P2P上限 dc_p2p_max","t1_trim_params.dc_p2p_max","uint16_t", "mV", 0, 65535, "0~65535");
    add("直流Trim最小 dc_trim_min","t1_trim_params.dc_trim_min","uint16_t", "mV", 0, 65535, "0~65535");
    add("直流Trim最大 dc_trim_max","t1_trim_params.dc_trim_max","uint16_t", "mV", 0, 65535, "0~65535");
    add("直流Trim目标 dc_trim_best","t1_trim_params.dc_trim_best","uint16_t", "mV", 0, 65535, "0~65535");
    add("脉冲频率最小 ac_trim_min","t1_trim_params.ac_trim_min","uint32_t", "Hz", 0, 2000000000ull, "示例: 1000");
    add("脉冲频率最大 ac_trim_max","t1_trim_params.ac_trim_max","uint32_t", "Hz", 0, 2000000000ull, "示例: 5000");
    add("脉冲频率目标 ac_trim_best","t1_trim_params.ac_trim_best","uint32_t", "Hz", 0, 2000000000ull, "示例: 3000");
    add("交流平均最小 ac_avg_min","t1_trim_params.ac_avg_min","uint16_t", "mV", 0, 65535, "0~65535");
    add("交流平均最大 ac_avg_max","t1_trim_params.ac_avg_max","uint16_t", "mV", 0, 65535, "0~65535");
    add("交流P2P最小 ac_p2p_min","t1_trim_params.ac_p2p_min","uint16_t", "mV", 0, 65535, "0~65535");
    add("交流P2P最大 ac_p2p_max","t1_trim_params.ac_p2p_max","uint16_t", "mV", 0, 65535, "0~65535");
    add("交流频率最小 ac_freq_min","t1_trim_params.ac_freq_min","uint32_t", "Hz", 0, 2000000000ull, "示例: 50");
    add("交流频率最大 ac_freq_max","t1_trim_params.ac_freq_max","uint32_t", "Hz", 0, 2000000000ull, "示例: 150");

    // delay_set
    add("上电延时 power_on_delay_ms","delay_set.power_on_delay_ms","uint32_t", "ms", 0, 2000000000ull, "示例: 100");
    add("DC稳定延时 t1_dc_stable_ms","delay_set.t1_dc_stable_ms","uint32_t", "ms", 0, 2000000000ull, "示例: 200");
    add("AC稳定延时 t1_ac_stable_ms","delay_set.t1_ac_stable_ms","uint32_t", "ms", 0, 2000000000ull, "示例: 200");
    add("烧录后延时 delay_after_program_ms","delay_set.delay_after_program_ms","uint32_t", "ms", 0, 2000000000ull, "示例: 500");
    add("寄存器操作延时 reg_operation_delay_us","delay_set.reg_operation_delay_us","uint32_t", "us", 0, 2000000000ull, "示例: 50");

    // 设置表格
    m_table->setRowCount(m_rowDefs.size());
    for (int i=0;i<m_rowDefs.size();++i){
        const auto& r=m_rowDefs[i];
        auto* c0=new QTableWidgetItem(r.zh); c0->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        auto* c1=new QTableWidgetItem(r.path); c1->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        auto* c2=new QTableWidgetItem(r.type); c2->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        auto* c3=new QTableWidgetItem("0");
        auto* c4=new QTableWidgetItem(rangeHint(r)); c4->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
        QString tip=rangeHint(r);
        c3->setToolTip(tip); c4->setToolTip(tip);
        m_table->setItem(i,0,c0);
        m_table->setItem(i,1,c1);
        m_table->setItem(i,2,c2);
        m_table->setItem(i,3,c3);
        m_table->setItem(i,4,c4);
    }
    validateAll();
}

QString Ag06TrimDialog::rangeHint(const RowDef& r) const
{
    QStringList hints;
    if (!r.unit.isEmpty()) hints << QString("单位:%1").arg(r.unit);
    if (r.maxV>r.minV || r.maxV==r.minV) hints << QString("范围:%1~%2").arg(r.minV).arg(r.maxV);
    if (!r.example.isEmpty()) hints << QString("示例:%1").arg(r.example);
    return hints.join(" | ");
}

void Ag06TrimDialog::validateRow(int row)
{
    if (m_isUpdating) return;
    if (row<0 || row>=m_rowDefs.size()) return;
    const auto& def=m_rowDefs[row];
    auto* cell=m_table->item(row,3);
    if (!cell) return;
    bool ok=false; quint64 v=cell->text().toULongLong(&ok,10);
    QString s = cell->text().trimmed();
    if (s.startsWith("0x", Qt::CaseInsensitive)) v=s.mid(2).toULongLong(&ok,16); else v=s.toULongLong(&ok,10);
    if (!ok) { cell->setBackground(QColor(255,220,220)); m_tableStatus->setText(QString("第%1行数值无效").arg(row+1)); return; }
    // 类型裁剪检查
    quint64 mask = def.type=="uint8_t"?0xFFull: def.type=="uint16_t"?0xFFFFull: def.type=="uint32_t"?0xFFFFFFFFull: ~0ull;
    if ((v & ~mask)!=0) { cell->setBackground(QColor(255,235,205)); }
    // 范围检查
    if (def.maxV>def.minV || def.maxV==def.minV){
        if (v<def.minV || v>def.maxV){ cell->setBackground(QColor(255,220,220)); m_tableStatus->setText(QString("第%1行超出范围[%2,%3]").arg(row+1).arg(def.minV).arg(def.maxV)); return; }
    }
    cell->setBackground(Qt::NoBrush);
    m_tableStatus->setText("就绪");
}

void Ag06TrimDialog::validateAll()
{
    m_isUpdating=true;
    for (int r=0;r<m_rowDefs.size();++r) validateRow(r);
    m_isUpdating=false;
}

 void Ag06TrimDialog::setupRegBitRow(QGridLayout* grid, int row, const QString& title, RegBitWidgets& w)
 {
     Q_UNUSED(title)
     w.addr = new QSpinBox; w.addr->setRange(0,255);
     w.start_bit = new QSpinBox; w.start_bit->setRange(0,31);
     w.width_bit = new QSpinBox; w.width_bit->setRange(0,32);
     w.write_back = new QSpinBox; w.write_back->setRange(0,1);
     grid->addWidget(new QLabel("addr"), row, 1); grid->addWidget(w.addr, row, 2);
     grid->addWidget(new QLabel("start"), row, 3); grid->addWidget(w.start_bit, row, 4);
     grid->addWidget(new QLabel("width"), row, 5); grid->addWidget(w.width_bit, row, 6);
     grid->addWidget(new QLabel("write_back"), row, 7); grid->addWidget(w.write_back, row, 8);
 }

 QJsonObject Ag06TrimDialog::buildTrimJson() const
 {
     QJsonObject obj;
     // 根据表格值组装 JSON
     auto getU=[&](const char* path){ return static_cast<int>(getCellUInt(path)); };
     // t1_trim_en
     QJsonObject en; en["value"]=getU("t1_trim_en.value"); obj["t1_trim_en"]=en;
     // regs
     auto reg=[&](const char* name){ QJsonObject r; r["addr"]=getU(QString("t1_trim_regs.%1.addr").arg(name).toUtf8().constData()); r["start_bit"]=getU(QString("t1_trim_regs.%1.start_bit").arg(name).toUtf8().constData()); r["width_bit"]=getU(QString("t1_trim_regs.%1.width_bit").arg(name).toUtf8().constData()); r["write_back"]=getU(QString("t1_trim_regs.%1.write_back").arg(name).toUtf8().constData()); return r; };
     QJsonObject regs; regs["output_ctrl"]=reg("output_ctrl"); regs["dc_trim"]=reg("dc_trim"); regs["ac_en"]=reg("ac_en"); regs["ac_trim"]=reg("ac_trim"); regs["eoc"]=reg("eoc"); obj["t1_trim_regs"]=regs;
     // 输出控制
     QJsonObject ocv; ocv["dc6"]=getU("t1_output_ctrl_value.dc6"); ocv["dc5"]=getU("t1_output_ctrl_value.dc5"); ocv["ac27"]=getU("t1_output_ctrl_value.ac27"); ocv["ac4"]=getU("t1_output_ctrl_value.ac4"); obj["t1_output_ctrl_value"]=ocv;
     // 参数
     QJsonObject p; p["icc_min"]=getU("t1_trim_params.icc_min"); p["icc_max"]=getU("t1_trim_params.icc_max"); p["dc_basic_min"]=getU("t1_trim_params.dc_basic_min"); p["dc_basic_max"]=getU("t1_trim_params.dc_basic_max"); p["dc_p2p_max"]=getU("t1_trim_params.dc_p2p_max"); p["dc_trim_min"]=getU("t1_trim_params.dc_trim_min"); p["dc_trim_max"]=getU("t1_trim_params.dc_trim_max"); p["dc_trim_best"]=getU("t1_trim_params.dc_trim_best"); p["ac_trim_min"]=getU("t1_trim_params.ac_trim_min"); p["ac_trim_max"]=getU("t1_trim_params.ac_trim_max"); p["ac_trim_best"]=getU("t1_trim_params.ac_trim_best"); p["ac_avg_min"]=getU("t1_trim_params.ac_avg_min"); p["ac_avg_max"]=getU("t1_trim_params.ac_avg_max"); p["ac_p2p_min"]=getU("t1_trim_params.ac_p2p_min"); p["ac_p2p_max"]=getU("t1_trim_params.ac_p2p_max"); p["ac_freq_min"]=getU("t1_trim_params.ac_freq_min"); p["ac_freq_max"]=getU("t1_trim_params.ac_freq_max"); obj["t1_trim_params"]=p;
     // 延时
     QJsonObject d; d["power_on_delay_ms"]=getU("delay_set.power_on_delay_ms"); d["t1_dc_stable_ms"]=getU("delay_set.t1_dc_stable_ms"); d["t1_ac_stable_ms"]=getU("delay_set.t1_ac_stable_ms"); d["delay_after_program_ms"]=getU("delay_set.delay_after_program_ms"); d["reg_operation_delay_us"]=getU("delay_set.reg_operation_delay_us"); obj["delay_set"]=d; return obj;
 }

 void Ag06TrimDialog::loadTrimJson(const QJsonObject& obj)
 {
    auto optObj=[&](const QJsonObject& o,const char* k){ return o.contains(k)?o.value(k).toObject():QJsonObject{}; };
    auto optInt=[&](const QJsonObject& o,const char* k){ return o.contains(k)?o.value(k).toInt():0; };
    // en
    m_isUpdating=true;
    setCellUInt("t1_trim_en.value", static_cast<quint64>(optObj(obj,"t1_trim_en").value("value").toInt()));
    // regs
    QJsonObject regs=obj.value("t1_trim_regs").toObject();
    auto putReg=[&](const char* name){ QJsonObject r=regs.value(name).toObject(); setCellUInt(QString("t1_trim_regs.%1.addr").arg(name).toUtf8().constData(), optInt(r,"addr")); setCellUInt(QString("t1_trim_regs.%1.start_bit").arg(name).toUtf8().constData(), optInt(r,"start_bit")); setCellUInt(QString("t1_trim_regs.%1.width_bit").arg(name).toUtf8().constData(), optInt(r,"width_bit")); setCellUInt(QString("t1_trim_regs.%1.write_back").arg(name).toUtf8().constData(), optInt(r,"write_back")); };
    putReg("output_ctrl"); putReg("dc_trim"); putReg("ac_en"); putReg("ac_trim"); putReg("eoc");
    // output ctrl
    QJsonObject ocv=obj.value("t1_output_ctrl_value").toObject(); setCellUInt("t1_output_ctrl_value.dc6", optInt(ocv,"dc6")); setCellUInt("t1_output_ctrl_value.dc5", optInt(ocv,"dc5")); setCellUInt("t1_output_ctrl_value.ac27", optInt(ocv,"ac27")); setCellUInt("t1_output_ctrl_value.ac4", optInt(ocv,"ac4"));
    // params
    QJsonObject p=obj.value("t1_trim_params").toObject();
    const char* keys[] = {"icc_min","icc_max","dc_basic_min","dc_basic_max","dc_p2p_max","dc_trim_min","dc_trim_max","dc_trim_best","ac_trim_min","ac_trim_max","ac_trim_best","ac_avg_min","ac_avg_max","ac_p2p_min","ac_p2p_max","ac_freq_min","ac_freq_max"};
    for (auto k: keys) setCellUInt(QString("t1_trim_params.%1").arg(k).toUtf8().constData(), optInt(p,k));
    // delay
    QJsonObject d=obj.value("delay_set").toObject(); const char* dk[] = {"power_on_delay_ms","t1_dc_stable_ms","t1_ac_stable_ms","delay_after_program_ms","reg_operation_delay_us"};
    for (auto k: dk) setCellUInt(QString("delay_set.%1").arg(k).toUtf8().constData(), optInt(d,k));
    m_isUpdating=false;
    validateAll();
 }

 xt_trim_t Ag06TrimDialog::collectTrim() const
 {
     xt_trim_t t{};
     // trim_en
     t.t1_trim_en.value = static_cast<uint32_t>(getCellUInt("t1_trim_en.value"));
     auto rb=[&](const char* name){ xt_reg_bit_t r{}; r.addr=static_cast<uint8_t>(getCellUInt(QString("t1_trim_regs.%1.addr").arg(name).toUtf8().constData())); r.start_bit=static_cast<uint8_t>(getCellUInt(QString("t1_trim_regs.%1.start_bit").arg(name).toUtf8().constData())); r.width_bit=static_cast<uint8_t>(getCellUInt(QString("t1_trim_regs.%1.width_bit").arg(name).toUtf8().constData())); r.write_back=static_cast<uint8_t>(getCellUInt(QString("t1_trim_regs.%1.write_back").arg(name).toUtf8().constData())); return r; };
     t.t1_trim_regs.output_ctrl=rb("output_ctrl"); t.t1_trim_regs.dc_trim=rb("dc_trim"); t.t1_trim_regs.ac_en=rb("ac_en"); t.t1_trim_regs.ac_trim=rb("ac_trim"); t.t1_trim_regs.eoc=rb("eoc");
     // 输出控制
     t.t1_output_ctrl_value.dc6=static_cast<uint8_t>(getCellUInt("t1_output_ctrl_value.dc6"));
     t.t1_output_ctrl_value.dc5=static_cast<uint8_t>(getCellUInt("t1_output_ctrl_value.dc5"));
     t.t1_output_ctrl_value.ac27=static_cast<uint8_t>(getCellUInt("t1_output_ctrl_value.ac27"));
     t.t1_output_ctrl_value.ac4=static_cast<uint8_t>(getCellUInt("t1_output_ctrl_value.ac4"));
     // 参数
     t.t1_trim_params.icc_min=static_cast<uint32_t>(getCellUInt("t1_trim_params.icc_min"));
     t.t1_trim_params.icc_max=static_cast<uint32_t>(getCellUInt("t1_trim_params.icc_max"));
     t.t1_trim_params.dc_basic_min=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_basic_min"));
     t.t1_trim_params.dc_basic_max=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_basic_max"));
     t.t1_trim_params.dc_p2p_max=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_p2p_max"));
     t.t1_trim_params.dc_trim_min=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_trim_min"));
     t.t1_trim_params.dc_trim_max=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_trim_max"));
     t.t1_trim_params.dc_trim_best=static_cast<uint16_t>(getCellUInt("t1_trim_params.dc_trim_best"));
     t.t1_trim_params.ac_trim_min=static_cast<uint32_t>(getCellUInt("t1_trim_params.ac_trim_min"));
     t.t1_trim_params.ac_trim_max=static_cast<uint32_t>(getCellUInt("t1_trim_params.ac_trim_max"));
     t.t1_trim_params.ac_trim_best=static_cast<uint32_t>(getCellUInt("t1_trim_params.ac_trim_best"));
     t.t1_trim_params.ac_avg_min=static_cast<uint16_t>(getCellUInt("t1_trim_params.ac_avg_min"));
     t.t1_trim_params.ac_avg_max=static_cast<uint16_t>(getCellUInt("t1_trim_params.ac_avg_max"));
     t.t1_trim_params.ac_p2p_min=static_cast<uint16_t>(getCellUInt("t1_trim_params.ac_p2p_min"));
     t.t1_trim_params.ac_p2p_max=static_cast<uint16_t>(getCellUInt("t1_trim_params.ac_p2p_max"));
     t.t1_trim_params.ac_freq_min=static_cast<uint32_t>(getCellUInt("t1_trim_params.ac_freq_min"));
     t.t1_trim_params.ac_freq_max=static_cast<uint32_t>(getCellUInt("t1_trim_params.ac_freq_max"));
     // 延时
     t.delay_set.power_on_delay_ms=static_cast<uint32_t>(getCellUInt("delay_set.power_on_delay_ms"));
     t.delay_set.t1_dc_stable_ms=static_cast<uint32_t>(getCellUInt("delay_set.t1_dc_stable_ms"));
     t.delay_set.t1_ac_stable_ms=static_cast<uint32_t>(getCellUInt("delay_set.t1_ac_stable_ms"));
     t.delay_set.delay_after_program_ms=static_cast<uint32_t>(getCellUInt("delay_set.delay_after_program_ms"));
     t.delay_set.reg_operation_delay_us=static_cast<uint32_t>(getCellUInt("delay_set.reg_operation_delay_us"));
     return t;
 }

int Ag06TrimDialog::findRow(const QString& path) const
{
    auto it=m_rowIndex.find(path);
    return it==m_rowIndex.end()? -1 : it.value();
}

QString Ag06TrimDialog::getCellText(const QString& path) const
{
    int r=findRow(path); if (r<0) return {};
    auto* it=m_table->item(r,3); return it?it->text():QString();
}

quint64 Ag06TrimDialog::getCellUInt(const QString& path) const
{
    int r=findRow(path); if (r<0) return 0;
    QString t=m_table->item(r,2)?m_table->item(r,2)->text():QString();
    QString v=m_table->item(r,3)?m_table->item(r,3)->text():QString("0");
    v = v.trimmed();
    bool ok=false; quint64 x=0;
    if (v.startsWith("0x", Qt::CaseInsensitive)) x=v.mid(2).toULongLong(&ok,16);
    else x=v.toULongLong(&ok,10);
    if (!ok) x=0;
    // 根据类型裁剪到位宽
    if (t=="uint8_t") x&=0xFFull; else if (t=="uint16_t") x&=0xFFFFull; else if (t=="uint32_t") x&=0xFFFFFFFFull;
    return x;
}

void Ag06TrimDialog::setCellText(const QString& path, const QString& text)
{
    int r=findRow(path); if (r<0) return; if (!m_table->item(r,3)) m_table->setItem(r,3,new QTableWidgetItem);
    m_table->item(r,3)->setText(text);
}

void Ag06TrimDialog::setCellUInt(const QString& path, quint64 v)
{
    int r=findRow(path); if (r<0) return; if (!m_table->item(r,3)) m_table->setItem(r,3,new QTableWidgetItem);
    m_table->item(r,3)->setText(QString::number(v));
}

