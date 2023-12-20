#include "Camera_calibration_module.h"

Camera_calibration_module::Camera_calibration_module(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	QAction* recentFileAction = nullptr;
	for (auto i = 0; i < maxFileNr; ++i) {
		recentFileAction = new QAction(this);
		recentFileAction->setVisible(false);
		connect(recentFileAction, &QAction::triggered, this, [=]()
			{
				open_project_fromstring(recentFileAction->data().toString());
			});
		recentFileActionList.append(recentFileAction);
	}

	recentFilesMenu = ui.menuProject->addMenu(tr("Open Recent"));
	for (auto i = 0; i < maxFileNr; ++i)
		recentFilesMenu->addAction(recentFileActionList.at(i));
	updateRecentActionList();
	connect(lin, &CLinguist::LanguageChaned, this, &Camera_calibration_module::RetranslateUI);
	QSettings settings;
	int recentLanguage = settings.value("Language").toInt();
	if (recentLanguage == Language::en_us)
	{
		LAN_NOW = Language::en_us;
		lin = CLinguist::GetLinguistInstance();
		lin->ChangeLanguage(Language::en_us);
		move_update_language();
	}
	if (recentLanguage == Language::zh_cn)
	{
		LAN_NOW = Language::zh_cn;
		lin = CLinguist::GetLinguistInstance();
		lin->ChangeLanguage(Language::zh_cn);
		move_update_language();
	}
	
	connect(ui.actionEnglish, SIGNAL(triggered()), this, SLOT(change_lan_en()));
	connect(ui.actionChinese, SIGNAL(triggered()), this, SLOT(change_lan_ch()));
	connect(ui.actionTraditional_Chinese, SIGNAL(triggered()), this, SLOT(change_lan_chc()));
	connect(ui.camera_number_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_cameras_and_groups()));
	connect(ui.group_number_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_cameras_and_groups()));
	connect(ui.curren_group_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_table_view_from_index()));
	connect(ui.curren_group_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.curren_index_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.show_detect_horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.show_detect_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.chess_width_size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.chess_height_size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(update_show_view()));
	connect(ui.calculate_pushButton, SIGNAL(clicked()), this, SLOT(calibration_cameras()));
	connect(ui.show_detect_horizontalSlider, SIGNAL(valueChanged(int)), ui.show_detect_spinBox, SLOT(setValue(int)));
	connect(ui.show_detect_spinBox, SIGNAL(valueChanged(int)), ui.show_detect_horizontalSlider, SLOT(setValue(int)));


	connect(ui.read_keypoint_pushButton, SIGNAL(clicked()), this, SLOT(select_path_read_keypoint()));
	connect(ui.clear_keypoint_pushButton, SIGNAL(clicked()), this, SLOT(clear_all_data()));
	connect(ui.export_pushButton, SIGNAL(clicked()), this, SLOT(export_inf_file()));
	connect(ui.actionResults, SIGNAL(triggered()), this, SLOT(select_path_read_result()));
	connect(ui.actionKeyPoints, SIGNAL(triggered()), this, SLOT(select_path_read_keypoint()));
	
	connect(ui.data_image_radioButton, SIGNAL(clicked()), this, SLOT(select_read_mode()));
	connect(ui.data_keypoint_radioButton, SIGNAL(clicked()), this, SLOT(select_read_mode()));
	connect(ui.camera_model_pushButton, SIGNAL(clicked()), this, SLOT(reset_camera_model()));

	connect(ui.ViewReport_pushButton, SIGNAL(clicked()), this, SLOT(view_report_inf()));

	connect(ui.reproject_error_limit_checkBox, SIGNAL(stateChanged(int)), this, SLOT(using_reporject_error_limit()));
	connect(ui.reproject_error_limit_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(update_table_view()));

	connect(ui.tableWidget, SIGNAL(cellChanged(int, int)), this, SLOT(update_checked_data(int, int)));
	
	connect(ui.show_plot_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update_showing_satck()));
	connect(ui.target_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(change_calibrate_target()));

	connect(ui.show_customplot, &QCustomPlot::mouseMove, this, &Camera_calibration_module::show_Histogram_data);


	connect(ui.fx_fy_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.cx_cy_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.k1_k2_k3_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.k4_k5_k6_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.p1_p2_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.s1_s2_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.s3_s4_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.Overall_R_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.Overall_T_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.shear_radioButton, SIGNAL(clicked()), this, SLOT(update_table_view()));
	connect(ui.verify_stable_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(update_table_view()));



	connect(ui.chess_width_size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.chess_height_size_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.chess_width_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.chess_height_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.offset_width_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.offset_height_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.circle_incre_width_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.circle_incre_height_size_oricircle_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.chess_width_length_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));
	connect(ui.chess_height_length_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));
	connect(ui.chess_height_length_oricircle_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));
	connect(ui.chess_width_length_oricircle_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));

	connect(ui.min_R_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.max_R_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.max_arc_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.max_KP_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.max_P_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.max_error_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));
	connect(ui.radius_k_doubleSpinBox, SIGNAL(valueChanged(double)), this, SLOT(save_params_cur()));
	connect(ui.max_ratio_spinBox, SIGNAL(valueChanged(int)), this, SLOT(save_params_cur()));
	connect(ui.target_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(save_params_cur()));


	init_box();
	
}

void Camera_calibration_module::read_params_cur()
{
	is_in_load = true;
	QSettings settings;
	if (settings.contains("PET_Cameras_Calibrate_Chess_Width"))
	{
		ui.chess_width_size_spinBox->setValue(settings.value("PET_Cameras_Calibrate_Chess_Width").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_Chess_Height"))
	{
		ui.chess_height_size_spinBox->setValue(settings.value("PET_Cameras_Calibrate_Chess_Height").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_Chess_LWidth"))
	{
		ui.chess_width_length_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_Chess_LWidth").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_Chess_LHeight"))
	{
		ui.chess_height_length_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_Chess_LHeight").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_Width"))
	{
		ui.chess_width_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_Width").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_Height"))
	{
		ui.chess_height_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_Height").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_LWidth"))
	{
		ui.chess_width_length_oricircle_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_LWidth").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_LHeight"))
	{
		ui.chess_height_length_oricircle_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_LHeight").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_OffWidth"))
	{
		ui.offset_width_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_OffWidth").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_OffHeight"))
	{
		ui.offset_height_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_OffHeight").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_IncreWidth"))
	{
		ui.circle_incre_width_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_IncreWidth").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_IncreHeight"))
	{
		ui.circle_incre_height_size_oricircle_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_IncreHeight").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_MinR"))
	{
		ui.min_R_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_MinR").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_MaxR"))
	{
		ui.max_R_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_MaxR").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_Minarc"))
	{
		ui.max_arc_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_Minarc").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_MinKP"))
	{
		ui.max_KP_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_MinKP").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_MinP"))
	{
		ui.max_P_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_MinP").toInt());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_MaxErr"))
	{
		ui.max_error_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_MaxErr").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_RadiusK"))
	{
		ui.radius_k_doubleSpinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_RadiusK").toDouble());
	}
	if (settings.contains("PET_Cameras_Calibrate_OriCircle_Maxratio"))
	{
		ui.max_ratio_spinBox->setValue(settings.value("PET_Cameras_Calibrate_OriCircle_Maxratio").toInt());
	}
	is_in_load = false;
}

void Camera_calibration_module::save_params_cur()
{
	if (is_in_load)
	{
		return;
	}
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		KeyPointNeedRecal_serial[ii] = true;
	}
	QSettings settings;
	settings.setValue("PET_Cameras_Calibrate_Chess_Width", ui.chess_width_size_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_Chess_Height", ui.chess_height_size_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_Chess_LWidth", ui.chess_width_length_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_Chess_LHeight", ui.chess_height_length_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_Width", ui.chess_width_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_Height", ui.chess_height_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_LWidth", ui.chess_width_length_oricircle_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_LHeight", ui.chess_height_length_oricircle_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_OffWidth", ui.offset_width_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_OffHeight", ui.offset_height_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_IncreWidth", ui.circle_incre_width_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_IncreHeight", ui.circle_incre_height_size_oricircle_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_MinR", ui.min_R_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_MaxR", ui.max_R_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_Minarc", ui.max_arc_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_MinKP", ui.max_KP_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_MinP", ui.max_P_spinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_MaxErr", ui.max_error_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_RadiusK", ui.radius_k_doubleSpinBox->value());
	settings.setValue("PET_Cameras_Calibrate_OriCircle_Maxratio", ui.max_ratio_spinBox->value());
	settings.sync();
}
void Camera_calibration_module::loop_sleep(int msec)
{
	QTime dieTime = QTime::currentTime().addMSecs(msec);

	while (QTime::currentTime() < dieTime) {
		QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}
}
void Camera_calibration_module::reset_camera_model()
{
	cam_model_ui->show();
}
void Camera_calibration_module::RetranslateUI()
{
	ui.retranslateUi(this);
}
void Camera_calibration_module::change_lan_ch()
{
	LAN_NOW = Language::zh_cn;
	lin = CLinguist::GetLinguistInstance();
	lin->ChangeLanguage(Language::zh_cn);
	move_update_language();

	QSettings settings;
	settings.setValue("Language", Language::zh_cn);
	settings.sync();
}
void Camera_calibration_module::change_lan_chc()
{
	QMessageBox::information(this, tr("Warning"), tr("Not support"), QMessageBox::Ok);
	return;
	lin = CLinguist::GetLinguistInstance();
	LAN_NOW = Language::zhc_cn;
	lin->ChangeLanguage(Language::zhc_cn);
	move_update_language();
	QSettings settings;
	settings.setValue("Language", Language::zhc_cn);
	settings.sync();
}
void Camera_calibration_module::change_lan_en()
{
	lin = CLinguist::GetLinguistInstance();
	LAN_NOW = Language::en_us;
	lin->ChangeLanguage(Language::en_us);
	move_update_language();
	QSettings settings;
	settings.setValue("Language", Language::en_us);
	settings.sync();
}

void Camera_calibration_module::move_update_language()
{
	recentFilesMenu->setTitle(tr("Open Recent"));
}

void Camera_calibration_module::using_reporject_error_limit()
{
	ui.reproject_error_limit_doubleSpinBox->setEnabled(ui.reproject_error_limit_checkBox->isChecked());
	update_table_view();
}

void Camera_calibration_module::updateRecentActionList()
{
	QSettings settings;
	QStringList recentFilePaths = settings.value("recentCalibFiles").toStringList();

	auto itEnd = 0;
	if (recentFilePaths.size() <= maxFileNr)
		itEnd = recentFilePaths.size();
	else
		itEnd = maxFileNr;

	for (auto i = 0; i < itEnd; ++i) {
		QString strippedName = QFileInfo(recentFilePaths.at(i)).fileName();
		recentFileActionList.at(i)->setText(strippedName);
		recentFileActionList.at(i)->setData(recentFilePaths.at(i));
		recentFileActionList.at(i)->setVisible(true);
	}

	for (auto i = itEnd; i < maxFileNr; ++i)
		recentFileActionList.at(i)->setVisible(false);

	settings.sync();
}

void Camera_calibration_module::adjustForCurrentFile(const QString& filePath) {
	currentFilePath = filePath;
	setWindowFilePath(currentFilePath);

	QSettings settings;
	QStringList recentFilePaths = settings.value("recentCalibFiles").toStringList();
	recentFilePaths.removeAll(filePath);
	recentFilePaths.prepend(filePath);
	while (recentFilePaths.size() > maxFileNr)
		recentFilePaths.removeLast();
	settings.setValue("recentCalibFiles", recentFilePaths);

	// see note
	updateRecentActionList();
}

void Camera_calibration_module::open_project_fromstring(QString fileNamed)
{
}
void QComboBox::wheelEvent(QWheelEvent* e)
{
}

Camera_calibration_module::~Camera_calibration_module()
{}

void Camera_calibration_module::init_box()
{
	read_params_cur();
	cam_model_ui = new Calibration_model_module();
	cam_model_ui->show();
	qApp->processEvents();
	cam_model_ui->hide();
	cam_log_ui = new Calibration_Log_module();
	cam_log_ui->show();
	qApp->processEvents();
	cam_log_ui->hide();
	sactter_view = new Q3DScatter();
	scatter_plot_view = QWidget::createWindowContainer(sactter_view);
	ui.gridLayout_for_scatter->addWidget(scatter_plot_view);

	if (sactter_dataArray != NULL)
	{
		delete sactter_dataArray;
	}
	sactter_dataArray = new QScatterDataArray();
	if (sactter_proxy == NULL)
	{
		sactter_proxy = new QScatterDataProxy();
	}
	if (sactter_series == NULL)
	{
		sactter_series = new QScatter3DSeries(sactter_proxy);
		sactter_view->addSeries(sactter_series);
	}
	sactter_series->setMeshSmooth(true);



	stable_sactter_view = new Q3DScatter();
	stable_3d_plot_view = QWidget::createWindowContainer(stable_sactter_view);
	ui.gridLayout_for_scatter_stable->addWidget(stable_3d_plot_view);

	if (stable_sactter_dataArray != NULL)
	{
		delete stable_sactter_dataArray;
	}
	stable_sactter_dataArray = new QScatterDataArray();
	if (stable_sactter_dataArray_real != NULL)
	{
		delete stable_sactter_dataArray_real;
	}
	stable_sactter_dataArray_real = new QScatterDataArray();
	if (stable_sactter_proxy == NULL)
	{
		stable_sactter_proxy = new QScatterDataProxy();
	}
	if (stable_sactter_proxy_real == NULL)
	{
		stable_sactter_proxy_real = new QScatterDataProxy();
	}
	if (stable_sactter_series == NULL)
	{
		stable_sactter_series = new QScatter3DSeries(stable_sactter_proxy);
		stable_sactter_view->addSeries(stable_sactter_series);
	}
	stable_sactter_series->setMeshSmooth(true);
	if (stable_sactter_series_real == NULL)
	{
		stable_sactter_series_real = new QScatter3DSeries(stable_sactter_proxy_real);
		stable_sactter_view->addSeries(stable_sactter_series_real);
	}
	stable_sactter_series_real->setMeshSmooth(true);

	ui.show_customplot->setInteraction(QCP::iRangeDrag, true);
	ui.show_customplot->setInteraction(QCP::iRangeZoom, true);
	ui.show_customplot->xAxis->setVisible(false);
	ui.show_customplot->yAxis->setVisible(false);
	ui.show_customplot->legend->setVisible(false);
	ui.show_customplot->plotLayout()->insertRow(0);
	title_customplot = new QCPTextElement(ui.show_customplot, "");
	ui.show_customplot->plotLayout()->addElement(0, 0, title_customplot);



	ui.show_stable_customplot->setInteraction(QCP::iRangeDrag, true);
	ui.show_stable_customplot->setInteraction(QCP::iRangeZoom, true);
	ui.show_stable_customplot->xAxis->setVisible(false);
	ui.show_stable_customplot->yAxis->setVisible(false);
	ui.show_stable_customplot->legend->setVisible(false);
	ui.show_stable_customplot->plotLayout()->insertRow(0);
	title_customplot_stable = new QCPTextElement(ui.show_stable_customplot, "");
	ui.show_stable_customplot->plotLayout()->addElement(0, 0, title_customplot_stable);

	Camera_box[0] = ui.camera_index_groupBox_1;
	Camera_box[1] = ui.camera_index_groupBox_2;
	Camera_box[2] = ui.camera_index_groupBox_3;
	Camera_box[3] = ui.camera_index_groupBox_4;
	Camera_box[4] = ui.camera_index_groupBox_5;
	Camera_box[5] = ui.camera_index_groupBox_6;
	Camera_box[6] = ui.camera_index_groupBox_7;
	Camera_box[7] = ui.camera_index_groupBox_8;
	Camera_box[8] = ui.camera_index_groupBox_9;
	Camera_box[9] = ui.camera_index_groupBox_10;
	Camera_box[10] = ui.camera_index_groupBox_11;
	Camera_box[11] = ui.camera_index_groupBox_12;
	Camera_box[12] = ui.camera_index_groupBox_13;
	Camera_box[13] = ui.camera_index_groupBox_14;
	Camera_box[14] = ui.camera_index_groupBox_15;
	Camera_box[15] = ui.camera_index_groupBox_16;
	Camera_box[16] = ui.camera_index_groupBox_17;
	Camera_box[17] = ui.camera_index_groupBox_18;
	Camera_box[18] = ui.camera_index_groupBox_19;
	Camera_box[19] = ui.camera_index_groupBox_20;
	Camera_box[20] = ui.camera_index_groupBox_21;
	Camera_box[21] = ui.camera_index_groupBox_22;
	Camera_box[22] = ui.camera_index_groupBox_23;
	Camera_box[23] = ui.camera_index_groupBox_24;
	Camera_box[24] = ui.camera_index_groupBox_25;
	Camera_box[25] = ui.camera_index_groupBox_26;
	Camera_box[26] = ui.camera_index_groupBox_27;
	Camera_box[27] = ui.camera_index_groupBox_28;
	Camera_box[28] = ui.camera_index_groupBox_29;
	Camera_box[29] = ui.camera_index_groupBox_30;
	Camera_box[30] = ui.camera_index_groupBox_31;
	Camera_box[31] = ui.camera_index_groupBox_32;
	Camera_box[32] = ui.camera_index_groupBox_33;
	Camera_box[33] = ui.camera_index_groupBox_34;
	Camera_box[34] = ui.camera_index_groupBox_35;
	Camera_box[35] = ui.camera_index_groupBox_36;

	Camera_label[0] = ui.camera_index_label_1;
	Camera_label[1] = ui.camera_index_label_2;
	Camera_label[2] = ui.camera_index_label_3;
	Camera_label[3] = ui.camera_index_label_4;
	Camera_label[4] = ui.camera_index_label_5;
	Camera_label[5] = ui.camera_index_label_6;
	Camera_label[6] = ui.camera_index_label_7;
	Camera_label[7] = ui.camera_index_label_8;
	Camera_label[8] = ui.camera_index_label_9;
	Camera_label[9] = ui.camera_index_label_10;
	Camera_label[10] = ui.camera_index_label_11;
	Camera_label[11] = ui.camera_index_label_12;
	Camera_label[12] = ui.camera_index_label_13;
	Camera_label[13] = ui.camera_index_label_14;
	Camera_label[14] = ui.camera_index_label_15;
	Camera_label[15] = ui.camera_index_label_16;
	Camera_label[16] = ui.camera_index_label_17;
	Camera_label[17] = ui.camera_index_label_18;
	Camera_label[18] = ui.camera_index_label_19;
	Camera_label[19] = ui.camera_index_label_20;
	Camera_label[20] = ui.camera_index_label_21;
	Camera_label[21] = ui.camera_index_label_22;
	Camera_label[22] = ui.camera_index_label_23;
	Camera_label[23] = ui.camera_index_label_24;
	Camera_label[24] = ui.camera_index_label_25;
	Camera_label[25] = ui.camera_index_label_26;
	Camera_label[26] = ui.camera_index_label_27;
	Camera_label[27] = ui.camera_index_label_28;
	Camera_label[28] = ui.camera_index_label_29;
	Camera_label[29] = ui.camera_index_label_30;
	Camera_label[30] = ui.camera_index_label_31;
	Camera_label[31] = ui.camera_index_label_32;
	Camera_label[32] = ui.camera_index_label_33;
	Camera_label[33] = ui.camera_index_label_34;
	Camera_label[34] = ui.camera_index_label_35;
	Camera_label[35] = ui.camera_index_label_36;

	Camera_combobox[0] = ui.camera_index_comboBox_1;
	Camera_combobox[1] = ui.camera_index_comboBox_2;
	Camera_combobox[2] = ui.camera_index_comboBox_3;
	Camera_combobox[3] = ui.camera_index_comboBox_4;
	Camera_combobox[4] = ui.camera_index_comboBox_5;
	Camera_combobox[5] = ui.camera_index_comboBox_6;
	Camera_combobox[6] = ui.camera_index_comboBox_7;
	Camera_combobox[7] = ui.camera_index_comboBox_8;
	Camera_combobox[8] = ui.camera_index_comboBox_9;
	Camera_combobox[9] = ui.camera_index_comboBox_10;
	Camera_combobox[10] = ui.camera_index_comboBox_11;
	Camera_combobox[11] = ui.camera_index_comboBox_12;
	Camera_combobox[12] = ui.camera_index_comboBox_13;
	Camera_combobox[13] = ui.camera_index_comboBox_14;
	Camera_combobox[14] = ui.camera_index_comboBox_15;
	Camera_combobox[15] = ui.camera_index_comboBox_16;
	Camera_combobox[16] = ui.camera_index_comboBox_17;
	Camera_combobox[17] = ui.camera_index_comboBox_18;
	Camera_combobox[18] = ui.camera_index_comboBox_19;
	Camera_combobox[19] = ui.camera_index_comboBox_20;
	Camera_combobox[20] = ui.camera_index_comboBox_21;
	Camera_combobox[21] = ui.camera_index_comboBox_22;
	Camera_combobox[22] = ui.camera_index_comboBox_23;
	Camera_combobox[23] = ui.camera_index_comboBox_24;
	Camera_combobox[24] = ui.camera_index_comboBox_25;
	Camera_combobox[25] = ui.camera_index_comboBox_26;
	Camera_combobox[26] = ui.camera_index_comboBox_27;
	Camera_combobox[27] = ui.camera_index_comboBox_28;
	Camera_combobox[28] = ui.camera_index_comboBox_29;
	Camera_combobox[29] = ui.camera_index_comboBox_30;
	Camera_combobox[30] = ui.camera_index_comboBox_31;
	Camera_combobox[31] = ui.camera_index_comboBox_32;
	Camera_combobox[32] = ui.camera_index_comboBox_33;
	Camera_combobox[33] = ui.camera_index_comboBox_34;
	Camera_combobox[34] = ui.camera_index_comboBox_35;
	Camera_combobox[35] = ui.camera_index_comboBox_36;

	Camera_pushbutton[0] = ui.camera_index_pushButton_1;
	Camera_pushbutton[1] = ui.camera_index_pushButton_2;
	Camera_pushbutton[2] = ui.camera_index_pushButton_3;
	Camera_pushbutton[3] = ui.camera_index_pushButton_4;
	Camera_pushbutton[4] = ui.camera_index_pushButton_5;
	Camera_pushbutton[5] = ui.camera_index_pushButton_6;
	Camera_pushbutton[6] = ui.camera_index_pushButton_7;
	Camera_pushbutton[7] = ui.camera_index_pushButton_8;
	Camera_pushbutton[8] = ui.camera_index_pushButton_9;
	Camera_pushbutton[9] = ui.camera_index_pushButton_10;
	Camera_pushbutton[10] = ui.camera_index_pushButton_11;
	Camera_pushbutton[11] = ui.camera_index_pushButton_12;
	Camera_pushbutton[12] = ui.camera_index_pushButton_13;
	Camera_pushbutton[13] = ui.camera_index_pushButton_14;
	Camera_pushbutton[14] = ui.camera_index_pushButton_15;
	Camera_pushbutton[15] = ui.camera_index_pushButton_16;
	Camera_pushbutton[16] = ui.camera_index_pushButton_17;
	Camera_pushbutton[17] = ui.camera_index_pushButton_18;
	Camera_pushbutton[18] = ui.camera_index_pushButton_19;
	Camera_pushbutton[19] = ui.camera_index_pushButton_20;
	Camera_pushbutton[20] = ui.camera_index_pushButton_21;
	Camera_pushbutton[21] = ui.camera_index_pushButton_22;
	Camera_pushbutton[22] = ui.camera_index_pushButton_23;
	Camera_pushbutton[23] = ui.camera_index_pushButton_24;
	Camera_pushbutton[24] = ui.camera_index_pushButton_25;
	Camera_pushbutton[25] = ui.camera_index_pushButton_26;
	Camera_pushbutton[26] = ui.camera_index_pushButton_27;
	Camera_pushbutton[27] = ui.camera_index_pushButton_28;
	Camera_pushbutton[28] = ui.camera_index_pushButton_29;
	Camera_pushbutton[29] = ui.camera_index_pushButton_30;
	Camera_pushbutton[30] = ui.camera_index_pushButton_31;
	Camera_pushbutton[31] = ui.camera_index_pushButton_32;
	Camera_pushbutton[32] = ui.camera_index_pushButton_33;
	Camera_pushbutton[33] = ui.camera_index_pushButton_34;
	Camera_pushbutton[34] = ui.camera_index_pushButton_35;
	Camera_pushbutton[35] = ui.camera_index_pushButton_36;



	Camera_clear_pushbutton[0] = ui.camera_index_clear_pushButton_1;
	Camera_clear_pushbutton[1] = ui.camera_index_clear_pushButton_2;
	Camera_clear_pushbutton[2] = ui.camera_index_clear_pushButton_3;
	Camera_clear_pushbutton[3] = ui.camera_index_clear_pushButton_4;
	Camera_clear_pushbutton[4] = ui.camera_index_clear_pushButton_5;
	Camera_clear_pushbutton[5] = ui.camera_index_clear_pushButton_6;
	Camera_clear_pushbutton[6] = ui.camera_index_clear_pushButton_7;
	Camera_clear_pushbutton[7] = ui.camera_index_clear_pushButton_8;
	Camera_clear_pushbutton[8] = ui.camera_index_clear_pushButton_9;
	Camera_clear_pushbutton[9] = ui.camera_index_clear_pushButton_10;
	Camera_clear_pushbutton[10] = ui.camera_index_clear_pushButton_11;
	Camera_clear_pushbutton[11] = ui.camera_index_clear_pushButton_12;
	Camera_clear_pushbutton[12] = ui.camera_index_clear_pushButton_13;
	Camera_clear_pushbutton[13] = ui.camera_index_clear_pushButton_14;
	Camera_clear_pushbutton[14] = ui.camera_index_clear_pushButton_15;
	Camera_clear_pushbutton[15] = ui.camera_index_clear_pushButton_16;
	Camera_clear_pushbutton[16] = ui.camera_index_clear_pushButton_17;
	Camera_clear_pushbutton[17] = ui.camera_index_clear_pushButton_18;
	Camera_clear_pushbutton[18] = ui.camera_index_clear_pushButton_19;
	Camera_clear_pushbutton[19] = ui.camera_index_clear_pushButton_20;
	Camera_clear_pushbutton[20] = ui.camera_index_clear_pushButton_21;
	Camera_clear_pushbutton[21] = ui.camera_index_clear_pushButton_22;
	Camera_clear_pushbutton[22] = ui.camera_index_clear_pushButton_23;
	Camera_clear_pushbutton[23] = ui.camera_index_clear_pushButton_24;
	Camera_clear_pushbutton[24] = ui.camera_index_clear_pushButton_25;
	Camera_clear_pushbutton[25] = ui.camera_index_clear_pushButton_26;
	Camera_clear_pushbutton[26] = ui.camera_index_clear_pushButton_27;
	Camera_clear_pushbutton[27] = ui.camera_index_clear_pushButton_28;
	Camera_clear_pushbutton[28] = ui.camera_index_clear_pushButton_29;
	Camera_clear_pushbutton[29] = ui.camera_index_clear_pushButton_30;
	Camera_clear_pushbutton[30] = ui.camera_index_clear_pushButton_31;
	Camera_clear_pushbutton[31] = ui.camera_index_clear_pushButton_32;
	Camera_clear_pushbutton[32] = ui.camera_index_clear_pushButton_33;
	Camera_clear_pushbutton[33] = ui.camera_index_clear_pushButton_34;
	Camera_clear_pushbutton[34] = ui.camera_index_clear_pushButton_35;
	Camera_clear_pushbutton[35] = ui.camera_index_clear_pushButton_36;


	Camera_lineedit[0] = ui.camera_index_lineEdit_1;
	Camera_lineedit[1] = ui.camera_index_lineEdit_2;
	Camera_lineedit[2] = ui.camera_index_lineEdit_3;
	Camera_lineedit[3] = ui.camera_index_lineEdit_4;
	Camera_lineedit[4] = ui.camera_index_lineEdit_5;
	Camera_lineedit[5] = ui.camera_index_lineEdit_6;
	Camera_lineedit[6] = ui.camera_index_lineEdit_7;
	Camera_lineedit[7] = ui.camera_index_lineEdit_8;
	Camera_lineedit[8] = ui.camera_index_lineEdit_9;
	Camera_lineedit[9] = ui.camera_index_lineEdit_10;
	Camera_lineedit[10] = ui.camera_index_lineEdit_11;
	Camera_lineedit[11] = ui.camera_index_lineEdit_12;
	Camera_lineedit[12] = ui.camera_index_lineEdit_13;
	Camera_lineedit[13] = ui.camera_index_lineEdit_14;
	Camera_lineedit[14] = ui.camera_index_lineEdit_15;
	Camera_lineedit[15] = ui.camera_index_lineEdit_16;
	Camera_lineedit[16] = ui.camera_index_lineEdit_17;
	Camera_lineedit[17] = ui.camera_index_lineEdit_18;
	Camera_lineedit[18] = ui.camera_index_lineEdit_19;
	Camera_lineedit[19] = ui.camera_index_lineEdit_20;
	Camera_lineedit[20] = ui.camera_index_lineEdit_21;
	Camera_lineedit[21] = ui.camera_index_lineEdit_22;
	Camera_lineedit[22] = ui.camera_index_lineEdit_23;
	Camera_lineedit[23] = ui.camera_index_lineEdit_24;
	Camera_lineedit[24] = ui.camera_index_lineEdit_25;
	Camera_lineedit[25] = ui.camera_index_lineEdit_26;
	Camera_lineedit[26] = ui.camera_index_lineEdit_27;
	Camera_lineedit[27] = ui.camera_index_lineEdit_28;
	Camera_lineedit[28] = ui.camera_index_lineEdit_29;
	Camera_lineedit[29] = ui.camera_index_lineEdit_30;
	Camera_lineedit[30] = ui.camera_index_lineEdit_31;
	Camera_lineedit[31] = ui.camera_index_lineEdit_32;
	Camera_lineedit[32] = ui.camera_index_lineEdit_33;
	Camera_lineedit[33] = ui.camera_index_lineEdit_34;
	Camera_lineedit[34] = ui.camera_index_lineEdit_35;
	Camera_lineedit[35] = ui.camera_index_lineEdit_36;

	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		connect(Camera_pushbutton[ii], SIGNAL(clicked()), this, SLOT(open_image_from_bmp()));
		connect(Camera_clear_pushbutton[ii], SIGNAL(clicked()), this, SLOT(clear_image_for_camera()));
		connect(Camera_combobox[ii], SIGNAL(currentIndexChanged(int)), this, SLOT(change_index_from()));
		Camera_combobox[ii]->clear();
		Camera_combobox[ii]->addItem(QString("%1").arg(1, 2, 10, QLatin1Char('0')));
		KeyPointNeedRecal_serial[ii] = true;
	}
	update_cameras_and_groups();
}

void Camera_calibration_module::update_cameras_and_groups()
{
	ui.verticalLayout_Left->setStretch(3, ui.camera_number_spinBox->value() <= 1 ? 1 : 2);
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		Camera_box[ii]->hide();
		if (Camera_combobox[ii]->count() != ui.group_number_spinBox->value())
		{
			int cur_index = Camera_combobox[ii]->currentIndex();
			Camera_combobox[ii]->clear();
			for (int jj = 0; jj < ui.group_number_spinBox->value(); jj++)
			{
				Camera_combobox[ii]->addItem(QString("%1").arg(jj + 1, 2, 10, QLatin1Char('0')));
			}
			if (cur_index >= ui.group_number_spinBox->value())
			{
				cur_index = ui.group_number_spinBox->value() - 1;
			}
			Camera_combobox[ii]->setCurrentIndex(cur_index);
		}
	}
	for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
	{
		Camera_box[ii]->show();
	}
	ui.curren_group_spinBox->setMinimum(1);
	ui.curren_index_spinBox->setMinimum(1);
	ui.curren_group_spinBox->setMaximum(ui.group_number_spinBox->value());
	ui.curren_index_spinBox->setMaximum(ui.camera_number_spinBox->value());
}

void Camera_calibration_module::select_read_mode()
{
	if (ui.data_image_radioButton->isChecked())
	{
		ui.load_data_stackedWidget->setCurrentIndex(0);
	}
	else if (ui.data_keypoint_radioButton->isChecked())
	{
		ui.load_data_stackedWidget->setCurrentIndex(1);
	}
}

void Camera_calibration_module::change_index_from()
{
	if (in_reading_keypoint)
	{
		return;
	}
	if (Result_K.size() != 0)
	{
		for (int ii = 0; ii < Result_K.size(); ii++)
		{
			delete[]Result_K[ii];
		}
	}
	if (Result_Dis_K.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K.size(); ii++)
		{
			delete[]Result_Dis_K[ii];
		}
	}
	if (Result_Dis_P.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P.size(); ii++)
		{
			delete[]Result_Dis_P[ii];
		}
	}
	if (Result_Dis_T.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T.size(); ii++)
		{
			delete[]Result_Dis_T[ii];
		}
	}
	Result_costdata_single.clear();
	Result_costdata_all.clear();
	Result_K.clear();
	Result_Dis_K.clear();
	Result_Dis_P.clear();
	Result_Dis_T.clear();
	Result_R.clear();
	Result_R_overall.clear();
	Result_T_overall.clear();
	Result_Re_map.clear();
	Result_re_err.clear();
	Result_update_calib_points.clear();
	Result_isenable.clear();
	Result_index.clear();
	if (Result_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_K_verify[ii].size(); jj++)
			{
				delete[] Result_K_verify[ii][jj];
			}
		}
	}
	Result_K_verify.clear();
	if (Result_Dis_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_K_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_K_verify[ii][jj];
			}
		}
	}
	Result_Dis_K_verify.clear();
	if (Result_Dis_P_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_P_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_P_verify[ii][jj];
			}
		}
	}
	Result_Dis_P_verify.clear();
	if (Result_Dis_T_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_T_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_T_verify[ii][jj];
			}
		}
	}
	Result_Dis_T_verify.clear();
	Result_R_overall_verify.clear();
	Result_T_overall_verify.clear();
	Result_update_calib_points_verify.clear();
	update_table_view();
}
void Camera_calibration_module::open_image_from_bmp()
{
	if (Result_K.size() != 0)
	{
		for (int ii = 0; ii < Result_K.size(); ii++)
		{
			delete[]Result_K[ii];
		}
	}
	if (Result_Dis_K.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K.size(); ii++)
		{
			delete[]Result_Dis_K[ii];
		}
	}
	if (Result_Dis_P.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P.size(); ii++)
		{
			delete[]Result_Dis_P[ii];
		}
	}
	if (Result_Dis_T.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T.size(); ii++)
		{
			delete[]Result_Dis_T[ii];
		}
	}
	Result_costdata_single.clear();
	Result_costdata_all.clear();
	Result_K.clear();
	Result_Dis_K.clear();
	Result_Dis_P.clear();
	Result_Dis_T.clear();
	Result_R.clear();
	Result_R_overall.clear();
	Result_T_overall.clear();
	Result_Re_map.clear();
	Result_re_err.clear();
	Result_update_calib_points.clear();
	Result_isenable.clear();
	Result_index.clear();
	if (Result_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_K_verify[ii].size(); jj++)
			{
				delete[] Result_K_verify[ii][jj];
			}
		}
	}
	Result_K_verify.clear();
	if (Result_Dis_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_K_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_K_verify[ii][jj];
			}
		}
	}
	Result_Dis_K_verify.clear();
	if (Result_Dis_P_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_P_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_P_verify[ii][jj];
			}
		}
	}
	Result_Dis_P_verify.clear();
	if (Result_Dis_T_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_T_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_T_verify[ii][jj];
			}
		}
	}
	Result_Dis_T_verify.clear();
	Result_R_overall_verify.clear();
	Result_T_overall_verify.clear();
	Result_update_calib_points_verify.clear();
	QPushButton* btn = (QPushButton*)sender();
	int cam_index = -1;
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		if (Camera_pushbutton[ii] == btn)
		{
			cam_index = ii;
			break;
		}
	}
	if (cam_index == -1)
	{
		return;
	}
	QString strs;
	QStringList file_list, output_name;
	QStringList str_path_list = QFileDialog::getOpenFileNames(this, tr("Select Images"), tr(""), tr("Image(*.bmp *.jpg *.tif *.png *.tiff *.jfif);;All(*)"));
	if (str_path_list.size() == 0)
	{
		return;
	}
	Image_serial_name[cam_index].clear();
	KeyPoint_Enable_serial[cam_index].clear();
	KeyPoint_serial[cam_index].clear();

	Result_contours_serial[cam_index].clear();
	KeyPointWidth_serial[cam_index].clear();
	KeyPointHeight_serial[cam_index].clear();
	ImageWidth_serial[cam_index].clear();
	ImageHeight_serial[cam_index].clear();
	KeyPointNeedRecal_serial[cam_index] = true;
	for (int ii = 0; ii < str_path_list.size(); ii++) 
	{
		Image_serial_name[cam_index].push_back(str_path_list[ii]);
		KeyPoint_Enable_serial[cam_index].push_back(true);
	}
	QDir file_temp = QDir(str_path_list[0]);
	file_temp.cdUp();
	Camera_lineedit[cam_index]->setText(file_temp.absolutePath());
	update_table_view();
	if (Camera_pushbutton[ui.curren_index_spinBox->value() - 1] == btn)
	{
		if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 0)
		{
			ui.show_detect_spinBox->setEnabled(false);
			ui.show_detect_horizontalSlider->setEnabled(false);
			ui.show_detect_spinBox->setMinimum(1);
			ui.show_detect_spinBox->setMaximum(1);
			ui.show_detect_horizontalSlider->setMinimum(1);
			ui.show_detect_horizontalSlider->setMaximum(1);
			ui.show_detect_spinBox->setValue(1);
			ui.show_detect_horizontalSlider->setValue(1);
			ui.Detect_view->clear_image();
		}
		else
		{
			ui.show_detect_spinBox->setEnabled(true);
			ui.show_detect_horizontalSlider->setEnabled(true);
			ui.show_detect_spinBox->setMinimum(1);
			ui.show_detect_spinBox->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
			ui.show_detect_horizontalSlider->setMinimum(1);
			ui.show_detect_horizontalSlider->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
			ui.show_detect_spinBox->setValue(1);
			ui.show_detect_horizontalSlider->setValue(1);
		}
		update_show_view();
	}
}

void Camera_calibration_module::clear_image_for_camera()
{
	if (Result_K.size() != 0)
	{
		for (int ii = 0; ii < Result_K.size(); ii++)
		{
			delete[]Result_K[ii];
		}
	}
	if (Result_Dis_K.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K.size(); ii++)
		{
			delete[]Result_Dis_K[ii];
		}
	}
	if (Result_Dis_P.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P.size(); ii++)
		{
			delete[]Result_Dis_P[ii];
		}
	}
	if (Result_Dis_T.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T.size(); ii++)
		{
			delete[]Result_Dis_T[ii];
		}
	}
	Result_costdata_single.clear();
	Result_costdata_all.clear();
	Result_K.clear();
	Result_Dis_K.clear();
	Result_Dis_P.clear();
	Result_Dis_T.clear();
	Result_R.clear();
	Result_R_overall.clear();
	Result_T_overall.clear();
	Result_Re_map.clear();
	Result_re_err.clear();
	Result_update_calib_points.clear();
	Result_isenable.clear();
	Result_index.clear();
	if (Result_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_K_verify[ii].size(); jj++)
			{
				delete[] Result_K_verify[ii][jj];
			}
		}
	}
	Result_K_verify.clear();
	if (Result_Dis_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_K_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_K_verify[ii][jj];
			}
		}
	}
	Result_Dis_K_verify.clear();
	if (Result_Dis_P_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_P_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_P_verify[ii][jj];
			}
		}
	}
	Result_Dis_P_verify.clear();
	if (Result_Dis_T_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_T_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_T_verify[ii][jj];
			}
		}
	}
	Result_Dis_T_verify.clear();
	Result_R_overall_verify.clear();
	Result_T_overall_verify.clear();
	Result_update_calib_points_verify.clear();
	QPushButton* btn = (QPushButton*)sender();
	int cam_index = -1;
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		if (Camera_clear_pushbutton[ii] == btn)
		{
			cam_index = ii;
			break;
		}
	}
	if (cam_index == -1)
	{
		return;
	}
	Image_serial_name[cam_index].clear();
	KeyPoint_Enable_serial[cam_index].clear();
	KeyPoint_serial[cam_index].clear();
	Result_contours_serial[cam_index].clear();
	KeyPointWidth_serial[cam_index].clear();
	KeyPointHeight_serial[cam_index].clear();
	ImageWidth_serial[cam_index].clear();
	ImageHeight_serial[cam_index].clear();
	KeyPointNeedRecal_serial[cam_index] = true;
	Camera_lineedit[cam_index]->setText("");
	update_table_view();
	if (Camera_clear_pushbutton[ui.curren_index_spinBox->value() - 1] == btn)
	{
		if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 0)
		{
			ui.show_detect_spinBox->setEnabled(false);
			ui.show_detect_horizontalSlider->setEnabled(false);
			ui.show_detect_spinBox->setMinimum(1);
			ui.show_detect_spinBox->setMaximum(1);
			ui.show_detect_horizontalSlider->setMinimum(1);
			ui.show_detect_horizontalSlider->setMaximum(1);
			ui.show_detect_spinBox->setValue(1);
			ui.show_detect_horizontalSlider->setValue(1);
			ui.Detect_view->clear_image();
		}
		else
		{
			ui.show_detect_spinBox->setEnabled(true);
			ui.show_detect_horizontalSlider->setEnabled(true);
			ui.show_detect_spinBox->setMinimum(1);
			ui.show_detect_spinBox->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
			ui.show_detect_horizontalSlider->setMinimum(1);
			ui.show_detect_horizontalSlider->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
			ui.show_detect_spinBox->setValue(1);
			ui.show_detect_horizontalSlider->setValue(1);
		}
		update_show_view();
	}
}
void Camera_calibration_module::change_calibrate_target()
{
	ui.target_stackedWidget->setCurrentIndex(ui.target_comboBox->currentIndex());
}
void Camera_calibration_module::update_showing_satck()
{
	std::vector<int> camera_index;
	std::vector<int> camera_valid_index;
	std::vector<int> camera_valid_number;
	switch (ui.show_plot_comboBox->currentIndex())
	{
	case 0:
		ui.plot_result_stackedWidget->setCurrentIndex(0);
		break;
	case 1:
		ui.plot_result_stackedWidget->setCurrentIndex(0);
		break;
	case 2:
		ui.plot_result_stackedWidget->setCurrentIndex(0);
		break;
	case 3:
		ui.plot_result_stackedWidget->setCurrentIndex(1);
		break;
	case 4:
		in_setting_verify = true;
		ui.plot_result_stackedWidget->setCurrentIndex(2);
		for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
		{
			if ((Camera_combobox[ii]->currentIndex() + 1) < 1 || (Camera_combobox[ii]->currentIndex() + 1) >= MAX_CALIBRAT_CAMERA)
			{
				continue;
			}
			if ((Camera_combobox[ii]->currentIndex() + 1) == ui.curren_group_spinBox->value())
			{
				camera_index.push_back(ii);
			}
		}

		for (int ii = 0; ii < camera_index.size(); ii++)
		{
			if (Image_serial_name[camera_index[ii]].size() != 0)
			{
				camera_valid_index.push_back(camera_index[ii]);
				camera_valid_number.push_back(Image_serial_name[camera_index[ii]].size());
			}
		}
		ui.verify_stable_comboBox->clear();
		for (int ii = 0; ii < camera_valid_index.size(); ii++)
		{
			ui.verify_stable_comboBox->addItem(tr("Camera-") + QString::number(camera_valid_index[ii] + 1));
		}
		in_setting_verify = false;
		break;
	default:
		break;
	}
	update_table_view();
}

void Camera_calibration_module::update_show_view()
{
	if (ui.curren_index_spinBox->value() < 1 || ui.curren_index_spinBox->value() >= MAX_CALIBRAT_CAMERA)
	{
		return;
	}
	if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 0)
	{
		ui.show_detect_spinBox->setEnabled(false);
		ui.show_detect_horizontalSlider->setEnabled(false);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(1);
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(1);
		ui.Detect_view->clear_image();
		ui.Detect_view->update();
		return;
	}
	else
	{
		ui.show_detect_spinBox->setEnabled(true);
		ui.show_detect_horizontalSlider->setEnabled(true);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
	}
	if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 1)
	{
		ui.show_detect_spinBox->setEnabled(false);
		ui.show_detect_horizontalSlider->setEnabled(false);
	}
	else
	{
		ui.show_detect_spinBox->setEnabled(true);
		ui.show_detect_horizontalSlider->setEnabled(true);
	}
	auto a = Image_serial_name[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1].toLocal8Bit().data();
	cv::Mat cur_img = cv::imread(Image_serial_name[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1].toLocal8Bit().data());

	if (cur_target_type_cal == Target_type::Chess_Board_type)
	{
		if ((ui.show_detect_spinBox->value() - 1) < KeyPoint_serial[ui.curren_index_spinBox->value() - 1].size())
		{
			if (KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] != 0 &&
				KeyPointHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] != 0)
			{
				if (ui.chess_width_size_spinBox->value() == 0 || ui.chess_height_size_spinBox->value() == 0 ||
					((ui.chess_width_size_spinBox->value() == KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]) &&
						(ui.chess_height_size_spinBox->value() == KeyPointHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]))
					)
				{
					if (cur_img.empty())
					{
						if (ImageWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] > 0 &&
							ImageHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] > 0)
						{
							cur_img = cv::Mat(ImageHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
								, ImageWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1], CV_8UC3, cv::Scalar(255, 255, 255));
						}
						else
						{
							ui.Detect_view->clear_image();
							ui.Detect_view->update();
							return;
						}
					}
					double all_number = 0;
					double draw_R_sum = 0;
					draw_R_sum = sqrt(
						(KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][0].x
							- KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][2].x)
						* (KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][1].x
							- KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][2].x)
						+ (KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][1].y
							- KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][2].y)
						* (KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][1].y
							- KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][2].y));

					double draw_R = draw_R_sum / 5;
					draw_R = draw_R < 5 ? 5 : draw_R;
					double draw_thick = draw_R / 2;
					draw_thick = draw_thick < 1 ? 1 : draw_thick;

					for (int pp = 0; pp < KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1].size(); pp++)
					{
						if (KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp].x != 0 &&
							KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp].y != 0)
						{
							circle(cur_img, KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp]
								, draw_R, cv::Scalar(0, 0, 255), draw_thick);
						}
					}
					arrowedLine(cur_img, KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][0]
						, KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] - 1],
						cv::Scalar(255, 0, 0), draw_thick, cv::LineTypes::LINE_8, 0, 0.5
						/ float(KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] - 1));
					

				}
				else
				{
					if (cur_img.empty())
					{
						ui.Detect_view->clear_image();
						ui.Detect_view->update();
						return;
					}
				}
			}
			else
			{
				if (cur_img.empty())
				{
					ui.Detect_view->clear_image();
					ui.Detect_view->update();
					return;
				}
			}
		}
		else
		{
			if (cur_img.empty())
			{
				ui.Detect_view->clear_image();
				ui.Detect_view->update();
				return;
			}
		}
	}
	else if (cur_target_type_cal == Target_type::Ori_Circle_Board_type)
	{
		if ((ui.show_detect_spinBox->value() - 1) < Result_contours_serial[ui.curren_index_spinBox->value() - 1].size())
		{
			if (KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] != 0 &&
				KeyPointHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] != 0)
			{
				if (ui.chess_width_size_spinBox->value() == 0 || ui.chess_height_size_spinBox->value() == 0 ||
					((ui.chess_width_size_spinBox->value() == KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]) &&
						(ui.chess_height_size_spinBox->value() == KeyPointHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]))
					)
				{
					if (cur_img.empty())
					{
						if (ImageWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] > 0 &&
							ImageHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] > 0)
						{
							cur_img = cv::Mat(ImageHeight_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
								, ImageWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1], CV_8UC3, cv::Scalar(255, 255, 255));
						}
						else
						{
							ui.Detect_view->clear_image();
							ui.Detect_view->update();
							return;
						}
					}
					double all_number = 0;
					double draw_R_sum = 0;
					auto p_1_temp = KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[(cur_target_offset_for_ori_hei - 1) * KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						+ cur_target_offset_for_ori_wid - 1];
					auto p_2_temp = KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[(cur_target_offset_for_ori_hei - 1) * KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						+ cur_target_offset_for_ori_wid + cur_target_incre_for_ori_wid - 1];
					auto p_3_temp = KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[(cur_target_offset_for_ori_hei + cur_target_incre_for_ori_hei - 1) * KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						+ cur_target_offset_for_ori_wid - 1];
					draw_R_sum += sqrt(pow(p_2_temp.x - p_1_temp.x, 2) + pow(p_2_temp.y - p_1_temp.y, 2));
					draw_R_sum += sqrt(pow(p_3_temp.x - p_1_temp.x, 2) + pow(p_3_temp.x - p_1_temp.y, 2));
					draw_R_sum /= (double)(cur_target_incre_for_ori_hei + cur_target_incre_for_ori_wid);
					double draw_R = draw_R_sum / 5;
					draw_R = draw_R < 5 ? 5 : draw_R;
					double draw_thick = draw_R / 2;
					draw_thick = draw_thick < 1 ? 1 : draw_thick;

					std::vector<std::vector<cv::Point>> Con_temp;
					for (int pp = 0; pp < Result_contours_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1].size(); pp++)
					{
						std::vector<cv::Point> C_tt;
						for (int qq = 0; qq < Result_contours_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp].size(); qq++)
						{
							C_tt.push_back(cv::Point(Result_contours_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp][qq].x,
								Result_contours_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1][pp][qq].y));
						}
						if (C_tt.size() != 0)
						{
							Con_temp.push_back(C_tt);
						}
					}
					cv::drawContours(cur_img, Con_temp, -1, cv::Scalar(0, 255, 0), draw_thick);
					arrowedLine(cur_img, KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[(cur_target_offset_for_ori_hei - 1) * KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						+ cur_target_offset_for_ori_wid - 1]
					, KeyPoint_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						[(cur_target_offset_for_ori_hei - 1) * KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1]
						+ cur_target_offset_for_ori_wid + cur_target_incre_for_ori_wid - 1],
						cv::Scalar(255, 0, 0), draw_thick, cv::LineTypes::LINE_8, 0, 0.5
						/ float(KeyPointWidth_serial[ui.curren_index_spinBox->value() - 1][ui.show_detect_spinBox->value() - 1] - 1));

				}
				else
				{
					if (cur_img.empty())
					{
						ui.Detect_view->clear_image();
						ui.Detect_view->update();
						return;
					}
				}
			}
			else
			{
				if (cur_img.empty())
				{
					ui.Detect_view->clear_image();
					ui.Detect_view->update();
					return;
				}
			}
		}
		else
		{
			if (cur_img.empty())
			{
				ui.Detect_view->clear_image();
				ui.Detect_view->update();
				return;
			}
		}
	}

	//ui.Detect_view->update();
	ui.Detect_view->cam_ratio = (double)cur_img.cols / (double)cur_img.rows;
	ui.Detect_view->set_image(QImage((const unsigned char*)cur_img.data, cur_img.cols,
		cur_img.rows, cur_img.step, QImage::Format_BGR888).copy());
	ui.Detect_view->update();

}


void Camera_calibration_module::update_checked_data(int rows, int cols)
{
	if (in_reading_keypoint)
	{
		return;
	}
	if (in_setting_keypoint_table)
	{
		return;
	}
	if (ui.camera_number_spinBox->value() < 1 || ui.camera_number_spinBox->value() >= MAX_CALIBRAT_CAMERA)
	{
		return;
	}
	std::vector<int> camera_index;
	for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
	{
		if ((Camera_combobox[ii]->currentIndex() + 1) < 1 || (Camera_combobox[ii]->currentIndex() + 1) >= MAX_CALIBRAT_CAMERA)
		{
			continue;
		}
		if ((Camera_combobox[ii]->currentIndex() + 1) == ui.curren_group_spinBox->value())
		{
			camera_index.push_back(ii);
		}
	}

	std::vector<int> camera_valid_index;
	std::vector<int> camera_valid_number;
	for (int ii = 0; ii < camera_index.size(); ii++)
	{
		if (Image_serial_name[camera_index[ii]].size() != 0)
		{
			camera_valid_index.push_back(camera_index[ii]);
			camera_valid_number.push_back(Image_serial_name[camera_index[ii]].size());
		}
	}
	if (camera_valid_index.size() == 0)
	{
		ui.tableWidget->setRowCount(0);
		return;
	}
	if (ui.tableWidget->item(rows, cols) == nullptr || ui.tableWidget->item(rows, cols)->text().isEmpty())
	{
		return;
	}
	int item_size = rows / camera_valid_index.size();
	if (ui.tableWidget->item(rows, cols)->checkState() == Qt::Checked)
	{
		for (int jj = 0; jj < camera_valid_index.size(); jj++)
		{
			if (item_size < KeyPoint_Enable_serial[camera_valid_index[jj]].size())
			{
				KeyPoint_Enable_serial[camera_valid_index[jj]][item_size] = true;
			}
			ui.tableWidget->item(item_size * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Checked);
		}
	}
	else if (ui.tableWidget->item(rows, cols)->checkState() == Qt::Unchecked)
	{
		for (int jj = 0; jj < camera_valid_index.size(); jj++)
		{
			if (item_size < KeyPoint_Enable_serial[camera_valid_index[jj]].size())
			{
				KeyPoint_Enable_serial[camera_valid_index[jj]][item_size] = false;
			}
			ui.tableWidget->item(item_size * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Unchecked);
		}
	}
}

void Camera_calibration_module::update_table_view_from_index()
{
	std::vector<int> camera_index;
	std::vector<int> camera_valid_index;
	std::vector<int> camera_valid_number;
	in_setting_verify = true;
	for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
	{
		if ((Camera_combobox[ii]->currentIndex() + 1) < 1 || (Camera_combobox[ii]->currentIndex() + 1) >= MAX_CALIBRAT_CAMERA)
		{
			continue;
		}
		if ((Camera_combobox[ii]->currentIndex() + 1) == ui.curren_group_spinBox->value())
		{
			camera_index.push_back(ii);
		}
	}

	for (int ii = 0; ii < camera_index.size(); ii++)
	{
		if (Image_serial_name[camera_index[ii]].size() != 0)
		{
			camera_valid_index.push_back(camera_index[ii]);
			camera_valid_number.push_back(Image_serial_name[camera_index[ii]].size());
		}
	}
	ui.verify_stable_comboBox->clear();
	for (int ii = 0; ii < camera_valid_index.size(); ii++)
	{
		ui.verify_stable_comboBox->addItem(tr("Camera-") + QString::number(camera_valid_index[ii] + 1));
	}
	in_setting_verify = false;
	update_table_view();
}
void Camera_calibration_module::update_table_view()
{
	if (in_reading_keypoint)
	{
		return;
	}
	if (in_setting_verify)
	{
		return;
	}
	//init draw
	ui.show_customplot->clearGraphs();
	ui.show_customplot->xAxis->setVisible(false);
	ui.show_customplot->yAxis->setVisible(false);
	ui.show_customplot->legend->setVisible(false);
	ui.show_customplot->xAxis2->setVisible(false);
	ui.show_customplot->yAxis2->setVisible(false);
	ui.show_customplot->xAxis2->setTickLabels(false);
	ui.show_customplot->yAxis2->setTickLabels(false);
	title_customplot->setText("");
	if (arrow_customplot != NULL)
	{
		arrow_customplot->setVisible(false);
	}
	if (ellipse_customplot != NULL)
	{
		ellipse_customplot->setVisible(false);
	}
	ui.show_customplot->xAxis->setTickLabels(false);
	ui.show_customplot->yAxis->setTickLabels(false);
	ui.show_customplot->clearPlottables();
	ui.show_customplot->replot(QCustomPlot::rpQueuedReplot);


	ui.show_stable_customplot->xAxis->setLabel("");
	ui.show_stable_customplot->yAxis->setLabel("");
	ui.show_stable_customplot->clearGraphs();
	ui.show_stable_customplot->xAxis->setVisible(false);
	ui.show_stable_customplot->yAxis->setVisible(false);
	ui.show_stable_customplot->legend->setVisible(false);
	ui.show_stable_customplot->xAxis2->setVisible(false);
	ui.show_stable_customplot->yAxis2->setVisible(false);
	ui.show_stable_customplot->xAxis2->setTickLabels(false);
	ui.show_stable_customplot->yAxis2->setTickLabels(false);
	title_customplot_stable->setText("");
	ui.show_stable_customplot->xAxis->setTickLabels(false);
	ui.show_stable_customplot->yAxis->setTickLabels(false);
	ui.show_stable_customplot->clearPlottables();
	ui.show_stable_customplot->replot(QCustomPlot::rpQueuedReplot);

	stable_sactter_proxy_real->arrayReset();
	stable_sactter_proxy->arrayReset();
	stable_sactter_dataArray->clear();
	stable_sactter_dataArray_real->clear();
	stable_sactter_proxy->resetArray(stable_sactter_dataArray);
	stable_sactter_proxy_real->resetArray(stable_sactter_dataArray_real);
	stable_sactter_view->setVisible(false);

	sactter_proxy->arrayReset();
	sactter_dataArray->clear();
	sactter_proxy->resetArray(sactter_dataArray);
	sactter_view->setVisible(false);

	if (ui.camera_number_spinBox->value() < 1 || ui.camera_number_spinBox->value() >= MAX_CALIBRAT_CAMERA)
	{
		return;
	}
	in_setting_keypoint_table = true;
	std::vector<int> camera_index;
	for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
	{
		if ((Camera_combobox[ii]->currentIndex() + 1) < 1 || (Camera_combobox[ii]->currentIndex() + 1) >= MAX_CALIBRAT_CAMERA)
		{
			continue;
		}
		if ((Camera_combobox[ii]->currentIndex() + 1) == ui.curren_group_spinBox->value())
		{
			camera_index.push_back(ii);
		}
	}

	std::vector<int> camera_valid_index;
	std::vector<int> camera_valid_number;
	for (int ii = 0; ii < camera_index.size(); ii++)
	{
		if (Image_serial_name[camera_index[ii]].size() != 0)
		{
			camera_valid_index.push_back(camera_index[ii]);
			camera_valid_number.push_back(Image_serial_name[camera_index[ii]].size());
		}
	}

	if (camera_valid_index.size() == 0)
	{
		ui.tableWidget->setRowCount(0);
		in_setting_keypoint_table = false;
		return;
	}
	bool in_group = false;
	for (int ii = 0; ii < camera_valid_number.size(); ii++)
	{
		if ((camera_valid_index[ii] + 1) == ui.curren_index_spinBox->value())
		{
			in_group = true;
			break;
		}
	}
	if (!in_group)
	{
		if ((camera_valid_index[0] + 1) <= ui.curren_index_spinBox->maximum() && (camera_valid_index[0] + 1) >= ui.curren_index_spinBox->minimum())
		{
			ui.curren_index_spinBox->setValue(camera_valid_index[0] + 1);
		}
	}
	int camera_max_size = camera_valid_number[0];
	int camera_min_size = camera_valid_number[0];
	for (int ii = 1; ii < camera_valid_number.size(); ii++)
	{
		if (camera_valid_number[ii] > camera_max_size)
		{
			camera_max_size = camera_valid_number[ii];
		}
		if (camera_valid_number[ii] < camera_min_size)
		{
			camera_min_size = camera_valid_number[ii];
		}
	}
	ui.tableWidget->setRowCount(0);
	ui.tableWidget->setRowCount(camera_max_size * camera_valid_index.size());
	for (int ii = 0; ii < camera_max_size; ii++)
	{
		for (int jj = 0; jj < camera_valid_index.size(); jj++)
		{
			if (ui.tableWidget->item(ii * camera_valid_index.size() + jj, 0) == NULL || 
				(ui.tableWidget->item(ii * camera_valid_index.size() + jj, 0) &&
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 0)->text() == tr("")))
			{
				if (ii >= Image_serial_name[camera_valid_index[jj]].size())
				{
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 0,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 1,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 2,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Unchecked);
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 3,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 4,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 5,
						new QTableWidgetItem(tr("-.-.-.")));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 6,
						new QTableWidgetItem(tr("-.-.-.")));
					QFont nullFont;
					QBrush nullColor(Qt::black);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setFont(nullFont);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setForeground(nullColor);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setFont(nullFont);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setForeground(nullColor);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setFont(nullFont);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setForeground(nullColor);
				}
				else
				{
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 0, 
						new QTableWidgetItem(QString("%1").arg(Camera_combobox[camera_valid_index[jj]]->currentIndex() + 1, 2, 10, QLatin1Char('0'))));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 1,
						new QTableWidgetItem(QString("%1").arg(camera_valid_index[jj] + 1, 2, 10, QLatin1Char('0'))));
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 2,
						new QTableWidgetItem(QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0'))));
					int has_found = -1;
					for (int ss = 0; ss < Result_index.size(); ss++)
					{
						if (Result_index[ss] == camera_valid_index[jj])
						{
							has_found = ss;
							break;
						}
					}
					if (has_found == -1 || ii >= Result_isenable[has_found].size() 
						|| !(Result_isenable[has_found][ii]) 
						||!(KeyPoint_Enable_serial[camera_valid_index[jj]][ii])
						|| has_found >= Result_re_err.size())
					{
						if (Result_isenable.size() != 0)
						{
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Unchecked);
						}
						else
						{
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Checked);
						}
						ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 3,
							new QTableWidgetItem(tr("--")));
						ui.tableWidget->setItem(ii* camera_valid_index.size() + jj, 4,
							new QTableWidgetItem(tr("--")));
						ui.tableWidget->setItem(ii* camera_valid_index.size() + jj, 5,
							new QTableWidgetItem(tr("--")));
						QFont nullFont;
						QBrush nullColor(Qt::black);
						ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setFont(nullFont);
						ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setForeground(nullColor);
						ui.tableWidget->item(ii* camera_valid_index.size() + jj, 4)->setFont(nullFont);
						ui.tableWidget->item(ii* camera_valid_index.size() + jj, 4)->setForeground(nullColor);
						ui.tableWidget->item(ii* camera_valid_index.size() + jj, 5)->setFont(nullFont);
						ui.tableWidget->item(ii* camera_valid_index.size() + jj, 5)->setForeground(nullColor);
					}
					else
					{
						double max_err = -std::numeric_limits<double>::max();
						std::vector<double> er_vec;
						for (int pp = 0; pp < Result_Re_map[has_found][ii].size(); pp++)
						{
							if (Result_Re_map[has_found][ii][pp].x == std::numeric_limits<float>::max()
								|| Result_Re_map[has_found][ii][pp].x == std::numeric_limits<float>::max())
							{
								continue;
							}
							double er_now_temp = sqrt(pow(Result_Re_map[has_found][ii][pp].x, 2) +
								pow(Result_Re_map[has_found][ii][pp].y, 2));
							er_vec.push_back(er_now_temp);
							max_err = max_err < er_now_temp ? er_now_temp : max_err;
						}
						size_t n_for_vec = er_vec.size() / 2;
						std::nth_element(er_vec.begin(), er_vec.begin() + n_for_vec, er_vec.end());
						float median_err = er_vec[n_for_vec];
						ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 3,
							new QTableWidgetItem(QString::number(Result_re_err[has_found][ii],'f',4)));

						ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 4,
							new QTableWidgetItem(QString::number(median_err, 'f', 4)));

						ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 5,
							new QTableWidgetItem(QString::number(max_err, 'f', 4)));

						ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Checked);
						if (ui.reproject_error_limit_doubleSpinBox->isEnabled()
							&& Result_re_err[has_found][ii] >= ui.reproject_error_limit_doubleSpinBox->value())
						{
							QFont nullFont;
							nullFont.setBold(true);
							QBrush nullColor(Qt::red);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setForeground(nullColor);
						}
						else
						{
							QFont nullFont;
							QBrush nullColor(Qt::black);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setForeground(nullColor);
						}
						if (ui.reproject_error_limit_doubleSpinBox->isEnabled()
							&& median_err >= ui.reproject_error_limit_doubleSpinBox->value())
						{
							QFont nullFont;
							nullFont.setBold(true);
							QBrush nullColor(Qt::red);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setForeground(nullColor);
						}
						else
						{
							QFont nullFont;
							QBrush nullColor(Qt::black);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setForeground(nullColor);
						}
						if (ui.reproject_error_limit_doubleSpinBox->isEnabled()
							&& max_err >= ui.reproject_error_limit_doubleSpinBox->value())
						{
							QFont nullFont;
							nullFont.setBold(true);
							QBrush nullColor(Qt::red);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setForeground(nullColor);
						}
						else
						{
							QFont nullFont;
							QBrush nullColor(Qt::black);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setFont(nullFont);
							ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setForeground(nullColor);
						}
					}
					ui.tableWidget->setItem(ii * camera_valid_index.size() + jj, 6, 
						new QTableWidgetItem(Image_serial_name[camera_valid_index[jj]][ii]));
				}
				if (camera_valid_index.size() == 1)
				{
					int cur_index = 255.0 / 2.0;
					cur_index = (cur_index < 0) ? 0 : cur_index;
					cur_index = (cur_index > 255) ? 255 : cur_index;
					auto back_color = QBrush(QColor(LOOKUPTABLE_JET[cur_index * 3] * 255,
						LOOKUPTABLE_JET[cur_index * 3 + 1] * 255, LOOKUPTABLE_JET[cur_index * 3 + 2] * 255, 80));
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 0)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 1)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 6)->setBackground(back_color);
				}
				else
				{
					int cur_index = 255.0 / (double)(camera_valid_index.size() - 1) * (double)jj;
					cur_index = (cur_index < 0) ? 0 : cur_index;
					cur_index = (cur_index > 255) ? 255 : cur_index;
					auto back_color = QBrush(QColor(LOOKUPTABLE_JET[cur_index * 3] * 255,
						LOOKUPTABLE_JET[cur_index * 3 + 1] * 255, LOOKUPTABLE_JET[cur_index * 3 + 2] * 255, 80));
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 0)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 1)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 3)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 4)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 5)->setBackground(back_color);
					ui.tableWidget->item(ii * camera_valid_index.size() + jj, 6)->setBackground(back_color);
				}
			}
		}
	}

	for (int ii = camera_min_size; ii < camera_max_size; ii++)
	{
		for (int jj = 0; jj < camera_valid_index.size(); jj++)
		{
			ui.tableWidget->item(ii * camera_valid_index.size() + jj, 2)->setCheckState(Qt::Unchecked);
		}
	}
	//绘图

	err_plot_showing.clear();
	err_plot_index_showing.clear();
	err_map_showing_x.clear();
	err_map_showing_y.clear();
	err_map_index_showing.clear();
	iteration_cost_showing_x.clear();
	iteration_cost_showing.clear(); 
	iteration_cost_index_showing.clear();
	double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
		, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
	double mean_x = 0, mean_y = 0, mean_R = 0;
	int number_fpr_data = 0 ;
	bool if_multicamera = false;
	switch (ui.show_plot_comboBox->currentIndex())
	{
	case 0:
		for (int ii = 0; ii < camera_valid_index.size(); ii++)
		{
			for (int jj = 0; jj < Result_index.size(); jj++)
			{
				if (camera_valid_index[ii] == Result_index[jj])
				{
					if (Result_re_err.size() <= jj)
					{
						continue;
					}
					err_plot_showing.push_back(Result_re_err[jj]);
					err_plot_index_showing.push_back(camera_valid_index[ii]);
					if (Result_re_err[jj].size() != 0)
					{
						err_plot_showing.at(err_plot_showing.size() - 1).pop_back();
					}
					break;
				}
			}
		}
		if (err_plot_showing.size() != 0)
		{
			std::vector<QCPBars*> bar_plot;
			QVector<double> ticks;
			QVector<QString> labels;
			if (err_plot_showing[0].size() <= 18)
			{
				for (int ii = 0; ii < err_plot_showing[0].size(); ii++)
				{
					ticks.push_back(ii + 1);
					labels.push_back(QString::number(ii + 1));
				}
			}
			else
			{
				for (int ii = 0; ii < 17; ii++)
				{
					ticks.push_back((int)((double)ii* (double)err_plot_showing[0].size() / 16.0) + 1);
					labels.push_back(QString::number((int)((double)ii* (double)err_plot_showing[0].size() / 16.0) + 1));
				}
			}
			QVector<double> ticks_all;
			for (int ii = 0; ii < err_plot_showing[0].size(); ii++)
			{
				ticks_all.push_back(ii + 1);
			}
			QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
			textTicker->addTicks(ticks, labels);
			ui.show_customplot->xAxis->setTicker(textTicker);

			QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
			ui.show_customplot->yAxis->setTicker(textTicker2);


			ui.show_customplot->xAxis->setScaleType(QCPAxis::stLinear);
			ui.show_customplot->yAxis->setScaleType(QCPAxis::stLinear);
			double max_err = 0;
			double mean_err = 0;
			int mean_err_num = 0;
			for (int ii = 0; ii < err_plot_showing.size(); ii++)
			{
				bar_plot.push_back(new QCPBars(ui.show_customplot->xAxis, ui.show_customplot->yAxis));
				bar_plot[ii]->setAntialiased(false); 
				bar_plot[ii]->setName(tr("Camera-") + QString::number(err_plot_index_showing[ii] + 1));

				int cur_index;
				if (err_plot_showing.size() == 1)
				{
					cur_index = 255.0 / 2.0;
				}
				else
				{
					cur_index = 255.0 / (double)(err_plot_showing.size() - 1) * (double)ii;
				}
				cur_index = (cur_index < 0) ? 0 : cur_index;
				cur_index = (cur_index > 255) ? 255 : cur_index;
				QBrush back_color = QBrush(QColor(LOOKUPTABLE_JET[cur_index * 3] * 255,
					LOOKUPTABLE_JET[cur_index * 3 + 1] * 255, LOOKUPTABLE_JET[cur_index * 3 + 2] * 255, 80));

				bar_plot[ii]->setPen(QPen(QColor(0, 0, 0))); 
				bar_plot[ii]->setBrush(back_color);
				QVector<double> bar_data;
				for (int jj = 0; jj < err_plot_showing[ii].size(); jj++)
				{
					bar_data.push_back(err_plot_showing[ii][jj]);
					if (err_plot_showing[ii][jj] != 0)
					{
						mean_err += err_plot_showing[ii][jj];
						mean_err_num++;
					}
					if (err_plot_showing[ii][jj] > max_err)
					{
						max_err = err_plot_showing[ii][jj];
					}
				}
				bar_plot[ii]->setData(ticks_all, bar_data);
			}
			mean_err /= (double)mean_err_num;
			QCPBarsGroup* bar_group = new QCPBarsGroup(ui.show_customplot);
			
			QList<QCPBars*> show_bars;
			for (int ii = 0; ii < bar_plot.size(); ii++)
			{
				show_bars.push_back(bar_plot[ii]);
			}
			foreach(QCPBars * bar_only, show_bars) 
			{
				bar_only->setWidthType(QCPBars::wtPlotCoords);
				bar_only->setWidth(bar_only->width() / show_bars.size());
				bar_group->append(bar_only);
			}
			arrow_customplot = new QCPItemLine(ui.show_customplot);
			arrow_customplot->start->setCoords(0.5, mean_err);
			arrow_customplot->end->setCoords(err_plot_showing[0].size() + 0.5, mean_err);
			QPen pen_bar;
			pen_bar.setWidth(1);
			pen_bar.setColor(Qt::red);
			pen_bar.setStyle(Qt::PenStyle::DashDotLine);
			arrow_customplot->setPen(pen_bar);

			cur_max_bar_showing = max_err;

			ui.show_customplot->xAxis->setVisible(true);
			ui.show_customplot->yAxis->setVisible(true);
			ui.show_customplot->xAxis->setTickLabels(true);
			ui.show_customplot->yAxis->setTickLabels(true);
			arrow_customplot->setVisible(true);
			ui.show_customplot->yAxis->setRange(0, max_err);
			ui.show_customplot->xAxis->setRange(0.5, err_plot_showing[0].size() + 1.5);
			title_customplot->setText(tr("Mean re-projection error = ") + QString::number(mean_err, 'f', 3));
		}
		break;
	case 1:
		for (int ii = 0; ii < camera_valid_index.size(); ii++)
		{
			for (int jj = 0; jj < Result_Re_map.size(); jj++)
			{
				if (camera_valid_index[ii] == Result_index[jj])
				{
					QVector<double> temp_err_x;
					QVector<double> temp_err_y;
					for (int pp = 0; pp < Result_Re_map[jj].size(); pp++)
					{
						for (int qq = 0; qq < Result_Re_map[jj][pp].size(); qq++)
						{
							temp_err_x.push_back(Result_Re_map[jj][pp][qq].x);
							temp_err_y.push_back(Result_Re_map[jj][pp][qq].y);
							if (Result_Re_map[jj][pp][qq].x > max_x)
							{
								max_x = Result_Re_map[jj][pp][qq].x;
							}
							if (Result_Re_map[jj][pp][qq].y > max_y)
							{
								max_y = Result_Re_map[jj][pp][qq].y;
							}
							if (Result_Re_map[jj][pp][qq].x < min_x)
							{
								min_x = Result_Re_map[jj][pp][qq].x;
							}
							if (Result_Re_map[jj][pp][qq].y < min_y)
							{
								min_y = Result_Re_map[jj][pp][qq].y;
							}
							if (Result_Re_map[jj][pp][qq].y != 0 && Result_Re_map[jj][pp][qq].x != 0)
							{
								mean_x += Result_Re_map[jj][pp][qq].x;
								mean_y += Result_Re_map[jj][pp][qq].y;
								number_fpr_data++;
							}
						}
					}
					err_map_showing_x.push_back(temp_err_x);
					err_map_showing_y.push_back(temp_err_y);
					err_map_index_showing.push_back(camera_valid_index[ii]);
					break;
				}
			}
		}
		if (err_map_showing_x.size() != 0)
		{
			mean_x /= (double)number_fpr_data;
			mean_y /= (double)number_fpr_data;
			for (int ii = 0; ii < camera_valid_index.size(); ii++)
			{
				for (int jj = 0; jj < Result_Re_map.size(); jj++)
				{
					if (ii == Result_index[jj])
					{
						for (int pp = 0; pp < Result_Re_map[jj].size(); pp++)
						{
							for (int qq = 0; qq < Result_Re_map[jj][pp].size(); qq++)
							{
								if (Result_Re_map[jj][pp][qq].y != 0 && Result_Re_map[jj][pp][qq].x != 0)
								{
									mean_R += sqrt(pow(Result_Re_map[jj][pp][qq].x - mean_x, 2) +
										pow(Result_Re_map[jj][pp][qq].y - mean_y, 2));
								}
							}
						}
					}
				}
			}
			mean_R /= (double)number_fpr_data;
			int graph_num = ui.show_customplot->graphCount();
			for (int ii = 0; ii < err_map_showing_x.size(); ii++)
			{
				ui.show_customplot->addGraph(ui.show_customplot->xAxis, ui.show_customplot->yAxis);
				int cur_index;
				if (err_map_index_showing.size() == 1)
				{
					cur_index = 255.0 / 2.0;
				}
				else
				{
					cur_index = 255.0 / (double)(err_map_index_showing.size() - 1) * (double)ii;
				}
				cur_index = (cur_index < 0) ? 0 : cur_index;
				cur_index = (cur_index > 255) ? 255 : cur_index;
				QColor back_color = QColor(LOOKUPTABLE_JET[cur_index * 3] * 255,
					LOOKUPTABLE_JET[cur_index * 3 + 1] * 255, LOOKUPTABLE_JET[cur_index * 3 + 2] * 255, 80);

				QPen pen_scatter;
				pen_scatter.setWidth(1);
				pen_scatter.setColor(back_color);
				pen_scatter.setStyle(Qt::PenStyle::SolidLine);
				ui.show_customplot->graph(ii)->setPen(pen_scatter);
				ui.show_customplot->graph(ii)->setLineStyle(QCPGraph::lsNone);
				ui.show_customplot->graph(ii)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 1));
				ui.show_customplot->graph(ii)->setName(tr("Camera-") + QString::number(err_map_index_showing[ii] + 1));
				ui.show_customplot->graph(ii)->setData(err_map_showing_x[ii], err_map_showing_y[ii]);
			}
			ellipse_customplot = new QCPItemEllipse(ui.show_customplot);
			//ellipse_customplot->start->setCoords(0.5, mean_err);
			//ellipse_customplot->end->setCoords(err_plot_showing[0].size() + 0.5, mean_err);
			QPen pen_scatter;
			pen_scatter.setWidth(1);
			pen_scatter.setColor(Qt::red);
			pen_scatter.setStyle(Qt::PenStyle::DashDotLine);
			ellipse_customplot->setPen(pen_scatter);
			ellipse_customplot->topLeft->setCoords(mean_x - mean_R, mean_y + mean_R);
			ellipse_customplot->bottomRight->setCoords(mean_x + mean_R, mean_y - mean_R);

			cur_max_x_scatter_showing = max_x;
			cur_max_y_scatter_showing = max_y;
			cur_min_x_scatter_showing = min_x;
			cur_min_y_scatter_showing = min_y;

			
			QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
			ui.show_customplot->xAxis->setTicker(textTicker);
			QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
			ui.show_customplot->yAxis->setTicker(textTicker2);

			ui.show_customplot->xAxis->setScaleType(QCPAxis::stLinear);
			ui.show_customplot->yAxis->setScaleType(QCPAxis::stLinear);

			ui.show_customplot->xAxis->setVisible(true);
			ui.show_customplot->yAxis->setVisible(true);
			for (int ii = 0; ii < ui.show_customplot->graphCount(); ii++)
			{
				if (ii == 0)
				{
					ui.show_customplot->graph(ii)->rescaleAxes();
				}
				else
				{
					ui.show_customplot->graph(ii)->rescaleAxes(true);
				}
			}
			ui.show_customplot->xAxis->setTickLabels(true);
			ui.show_customplot->yAxis->setTickLabels(true);
			ellipse_customplot->setVisible(true);
			title_customplot->setText(tr("Average circle radius = ") + QString::number(mean_R, 'f', 3) + tr("; Center = (") 
				+ QString::number(mean_x, 'f', 3) + tr(", ") + QString::number(mean_y, 'f', 3) + tr(")"));
		}
		break;
	case 2:
		for (int ii = 0; ii < camera_valid_index.size(); ii++)
		{
			for (int jj = 0; jj < Result_index.size(); jj++)
			{
				if (camera_valid_index[ii] == Result_index[jj])
				{
					QVector<double> temp_cost_x;
					QVector<double> temp_cost;
					for (int pp = 0; pp < Result_costdata_single[jj].size(); pp++)
					{
						temp_cost_x.push_back(pp + 1);
						temp_cost.push_back(Result_costdata_single[jj][pp]);
						if (Result_costdata_single[jj][pp] > max_x)
						{
							max_x = Result_costdata_single[jj][pp];
						}
						if (Result_costdata_single[jj][pp] < min_x)
						{
							min_x = Result_costdata_single[jj][pp];
						}
					}
					iteration_cost_showing_x.push_back(temp_cost_x);
					iteration_cost_showing.push_back(temp_cost);
					iteration_cost_index_showing.push_back(camera_valid_index[ii]);
				}
			}
		}
		if (iteration_cost_showing.size() != 0)
		{
			if (iteration_cost_showing.size() > 1)
			{
				bool if_get = false;
				for (int ii = 0; ii < camera_valid_index.size(); ii++)
				{
					for (int jj = 0; jj < Result_index.size(); jj++)
					{
						if (ii == Result_index[jj])
						{
							QVector<double> temp_cost_x;
							QVector<double> temp_cost;
							if (Result_costdata_all[jj].size() == 0)
							{
								continue;
							}
							for (int pp = 0; pp < Result_costdata_all[jj].size(); pp++)
							{
								temp_cost_x.push_back(pp + 1);
								temp_cost.push_back(Result_costdata_all[jj][pp]);
								if (Result_costdata_all[jj][pp] > max_x)
								{
									max_x = Result_costdata_all[jj][pp];
								}
								if (Result_costdata_all[jj][pp] < min_x)
								{
									min_x = Result_costdata_all[jj][pp];
								}
							}
							iteration_cost_showing_x.push_back(temp_cost_x);
							iteration_cost_showing.push_back(temp_cost);
							if_get = true;
						}
						if (if_get)
						{
							break;
						}
					}
					if (if_get)
					{
						break;
					}
				}
				if_multicamera = true;
			}
			else
			{
				if_multicamera = false;
			}
			int graph_num = ui.show_customplot->graphCount();
			for (int ii = 0; ii < iteration_cost_showing.size(); ii++)
			{
				ui.show_customplot->addGraph(ui.show_customplot->xAxis, ui.show_customplot->yAxis);
				int cur_index;
				if (!if_multicamera)
				{
					if (iteration_cost_showing.size() == 1)
					{
						cur_index = 255.0 / 2.0;
					}
					else
					{
						cur_index = 255.0 / (double)(iteration_cost_showing.size() - 1) * (double)ii;
					}
					cur_index = (cur_index < 0) ? 0 : cur_index;
					cur_index = (cur_index > 255) ? 255 : cur_index;
				}
				else
				{
					if (iteration_cost_showing.size() == 2)
					{
						cur_index = 255.0 / 2.0;
					}
					else
					{
						cur_index = 255.0 / (double)(iteration_cost_showing.size() - 2) * (double)ii;
						cur_index = (cur_index < 0) ? 0 : cur_index;
						cur_index = (cur_index > 255) ? 255 : cur_index;
					}
				}
				QColor back_color = QColor(LOOKUPTABLE_JET[cur_index * 3] * 255,
					LOOKUPTABLE_JET[cur_index * 3 + 1] * 255, LOOKUPTABLE_JET[cur_index * 3 + 2] * 255, 80);

				if (if_multicamera && ii == (iteration_cost_showing.size() - 1))
				{
					back_color = QColor(Qt::red);
				}
				QPen pen_scatter;
				pen_scatter.setWidth(3);
				pen_scatter.setColor(back_color);
				pen_scatter.setStyle(Qt::PenStyle::SolidLine);

				ui.show_customplot->graph(ii)->setPen(pen_scatter);
				ui.show_customplot->graph(ii)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
				if (if_multicamera && ii == (iteration_cost_showing.size() - 1))
				{
					ui.show_customplot->graph(ii)->setName(tr("Multi-Cameras"));
				}
				else
				{
					ui.show_customplot->graph(ii)->setName(tr("Camera-") + QString::number(iteration_cost_index_showing[ii] + 1));
				}
				ui.show_customplot->graph(ii)->setData(iteration_cost_showing_x[ii], iteration_cost_showing[ii]);
			}

			QSharedPointer<QCPAxisTickerLog> textTicker(new QCPAxisTickerLog);
			ui.show_customplot->yAxis->setTicker(textTicker);
			ui.show_customplot->yAxis->setScaleType(QCPAxis::stLogarithmic);

			QSharedPointer<QCPAxisTicker> textTicker3(new QCPAxisTicker);
			ui.show_customplot->xAxis->setTicker(textTicker3);

			ui.show_customplot->xAxis->setVisible(true);
			ui.show_customplot->yAxis->setVisible(true);
			ui.show_customplot->xAxis->setTickLabels(true);
			ui.show_customplot->yAxis->setTickLabels(true);
			ui.show_customplot->xAxis->rescale();
			ui.show_customplot->yAxis->setRange(min_x, max_x);
			title_customplot->setText(tr("Iterative cost convergence"));
		}
		break;
	case 3:
	{
		ui.plot_result_stackedWidget->setCurrentIndex(1);
		sactter_view->setVisible(true);
		sactter_view->axisX()->setTitle("X/mm");
		sactter_view->axisX()->setTitleVisible(true);
		sactter_view->axisY()->setTitle("Y/mm");
		sactter_view->axisY()->setTitleVisible(true);
		sactter_view->axisZ()->setTitle("Z/mm");
		sactter_view->axisZ()->setTitleVisible(true);
		sactter_view->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
		sactter_view->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);

		for (int ii = 0; ii < camera_valid_index.size(); ii++)
		{
			for (int jj = 0; jj < Result_index.size(); jj++)
			{
				if (camera_valid_index[ii] == Result_index[jj])
				{
					for (int pp = 0; pp < Result_update_calib_points[jj].size(); pp++)
					{
						*sactter_dataArray << QVector3D(Result_update_calib_points[jj][pp].x,
							Result_update_calib_points[jj][pp].y,
							Result_update_calib_points[jj][pp].z);
					}
					sactter_series->setBaseColor(Qt::blue);
					sactter_proxy->resetArray(sactter_dataArray);
					sactter_view->setTitle(tr("(Updated) calibration keypoints"));
					sactter_view->setAspectRatio(1.0);
					sactter_view->setHorizontalAspectRatio(1.0);
					break;
				}
			}
		}

	}
		break;
	case 4:
	{
		if (ui.verify_stable_comboBox->currentIndex() >= camera_valid_index.size())
		{
			set_verify_showing_checked(false);
			set_verify_showing_enbale(false);
			break;
		}
		int stable_verify_index = -1;
		for (int jj = 0; jj < Result_index.size(); jj++)
		{
			if (camera_valid_index[ui.verify_stable_comboBox->currentIndex()] == Result_index[jj])
			{
				stable_verify_index = jj;
			}
		}
		if (stable_verify_index == -1)
		{
			set_verify_showing_checked(false);
			set_verify_showing_enbale(false);
			break;
		}
		if (Result_K_verify[stable_verify_index].size() == 0)
		{
			set_verify_showing_checked(false);
			set_verify_showing_enbale(false);
			break;
		}
		if ((!ui.fx_fy_radioButton->isChecked()) && (!ui.cx_cy_radioButton->isChecked()) && (!ui.k1_k2_k3_radioButton->isChecked()) &&
			(!ui.k4_k5_k6_radioButton->isChecked()) && (!ui.p1_p2_radioButton->isChecked()) && (!ui.s1_s2_radioButton->isChecked()) &&
			(!ui.s3_s4_radioButton->isChecked()) && (!ui.Overall_R_radioButton->isChecked()) && (!ui.Overall_T_radioButton->isChecked()) &&
			(!ui.shear_radioButton->isChecked()))
		{
			ui.fx_fy_radioButton->setChecked(true);
		}
		draw_stable_plot(stable_verify_index);
	}
		break;
	default:
		break;
	}
	ui.show_customplot->replot(QCustomPlot::rpQueuedReplot);
	in_setting_keypoint_table = false;
}

void Camera_calibration_module::draw_stable_plot(int cur_index)
{
	if (cur_index >= Result_K_verify.size())
	{
		return;
	}
	if (Result_K_verify[cur_index].size() == 0)
	{
		return;
	}
	if (ui.fx_fy_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, std_x = 0, std_y = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			value_x.push_back(Result_K_verify[cur_index][ii][0]);
			value_y.push_back(Result_K_verify[cur_index][ii][1]);
			mean_x += Result_K_verify[cur_index][ii][0];
			mean_y += Result_K_verify[cur_index][ii][1];
			num_xy++;
			if (Result_K_verify[cur_index][ii][0] > max_x)
			{
				max_x = Result_K_verify[cur_index][ii][0];
			}
			if (Result_K_verify[cur_index][ii][1] > max_y)
			{
				max_y = Result_K_verify[cur_index][ii][1];
			}
			if (Result_K_verify[cur_index][ii][0] < min_x)
			{
				min_x = Result_K_verify[cur_index][ii][0];
			}
			if (Result_K_verify[cur_index][ii][1] < min_y)
			{
				min_y = Result_K_verify[cur_index][ii][1];
			}
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_K_verify[cur_index][ii][0] - mean_x, 2);
			std_y += pow(Result_K_verify[cur_index][ii][1] - mean_y, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_K.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(Result_K[cur_index][0]);
			value_y_all.push_back(Result_K[cur_index][1]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_K[cur_index][0] > max_x)
			{
				max_x = Result_K[cur_index][0];
			}
			if (Result_K[cur_index][1] > max_y)
			{
				max_y = Result_K[cur_index][1];
			}
			if (Result_K[cur_index][0] < min_x)
			{
				min_x = Result_K[cur_index][0];
			}
			if (Result_K[cur_index][1] < min_y)
			{
				min_y = Result_K[cur_index][1];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);

		ui.show_stable_customplot->xAxis->setLabel("Fx");
		ui.show_stable_customplot->yAxis->setLabel("Fy");
		ui.show_stable_customplot->xAxis->setVisible(true);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(true);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}
		if (max_y != min_y)
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 0.1 * (max_y - min_y), min_y + 1.5 * (max_y - min_y));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 1e-4, min_y + 1e-4);
		}
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n Fx: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) + 
			tr(") / Fy: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
	}
	else if (ui.cx_cy_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, std_x = 0, std_y = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			value_x.push_back(Result_K_verify[cur_index][ii][2]);
			value_y.push_back(Result_K_verify[cur_index][ii][3]);
			mean_x += Result_K_verify[cur_index][ii][2];
			mean_y += Result_K_verify[cur_index][ii][3];
			num_xy++;
			if (Result_K_verify[cur_index][ii][2] > max_x)
			{
				max_x = Result_K_verify[cur_index][ii][2];
			}
			if (Result_K_verify[cur_index][ii][3] > max_y)
			{
				max_y = Result_K_verify[cur_index][ii][3];
			}
			if (Result_K_verify[cur_index][ii][2] < min_x)
			{
				min_x = Result_K_verify[cur_index][ii][2];
			}
			if (Result_K_verify[cur_index][ii][3] < min_y)
			{
				min_y = Result_K_verify[cur_index][ii][3];
			}
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_K_verify[cur_index][ii][2] - mean_x, 2);
			std_y += pow(Result_K_verify[cur_index][ii][3] - mean_y, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_K.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(Result_K[cur_index][2]);
			value_y_all.push_back(Result_K[cur_index][3]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_K[cur_index][2] > max_x)
			{
				max_x = Result_K[cur_index][2];
			}
			if (Result_K[cur_index][3] > max_y)
			{
				max_y = Result_K[cur_index][3];
			}
			if (Result_K[cur_index][2] < min_x)
			{
				min_x = Result_K[cur_index][2];
			}
			if (Result_K[cur_index][3] < min_y)
			{
				min_y = Result_K[cur_index][3];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);

		ui.show_stable_customplot->xAxis->setLabel("Cx");
		ui.show_stable_customplot->yAxis->setLabel("Cy");
		ui.show_stable_customplot->xAxis->setVisible(true);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(true);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}
		if (max_y != min_y)
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 0.1 * (max_y - min_y), min_y + 1.5 * (max_y - min_y));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 1e-4, min_y + 1e-4);
		}
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n Cx: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / Cy: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
	}
	else if (ui.p1_p2_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, std_x = 0, std_y = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_Dis_P_verify[cur_index].size(); ii++)
		{
			value_x.push_back(Result_Dis_P_verify[cur_index][ii][0]);
			value_y.push_back(Result_Dis_P_verify[cur_index][ii][1]);
			mean_x += Result_Dis_P_verify[cur_index][ii][0];
			mean_y += Result_Dis_P_verify[cur_index][ii][1];
			num_xy++;
			if (Result_Dis_P_verify[cur_index][ii][0] > max_x)
			{
				max_x = Result_Dis_P_verify[cur_index][ii][0];
			}
			if (Result_Dis_P_verify[cur_index][ii][1] > max_y)
			{
				max_y = Result_Dis_P_verify[cur_index][ii][1];
			}
			if (Result_Dis_P_verify[cur_index][ii][0] < min_x)
			{
				min_x = Result_Dis_P_verify[cur_index][ii][0];
			}
			if (Result_Dis_P_verify[cur_index][ii][1] < min_y)
			{
				min_y = Result_Dis_P_verify[cur_index][ii][1];
			}
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		for (int ii = 0; ii < Result_Dis_P_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_Dis_P_verify[cur_index][ii][0] - mean_x, 2);
			std_y += pow(Result_Dis_P_verify[cur_index][ii][1] - mean_y, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_Dis_P.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(Result_Dis_P[cur_index][0]);
			value_y_all.push_back(Result_Dis_P[cur_index][1]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_Dis_P[cur_index][0] > max_x)
			{
				max_x = Result_Dis_P[cur_index][0];
			}
			if (Result_Dis_P[cur_index][1] > max_y)
			{
				max_y = Result_Dis_P[cur_index][1];
			}
			if (Result_Dis_P[cur_index][0] < min_x)
			{
				min_x = Result_Dis_P[cur_index][0];
			}
			if (Result_Dis_P[cur_index][1] < min_y)
			{
				min_y = Result_Dis_P[cur_index][1];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);

		ui.show_stable_customplot->xAxis->setLabel("P1");
		ui.show_stable_customplot->yAxis->setLabel("P2");
		ui.show_stable_customplot->xAxis->setVisible(true);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(true);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}
		if (max_y != min_y)
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 0.1 * (max_y - min_y), min_y + 1.5 * (max_y - min_y));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 1e-4, min_y + 1e-4);
		}
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n P1: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / P2: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
	}
	else if (ui.s1_s2_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, std_x = 0, std_y = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			value_x.push_back(Result_Dis_T_verify[cur_index][ii][0]);
			value_y.push_back(Result_Dis_T_verify[cur_index][ii][1]);
			mean_x += Result_Dis_T_verify[cur_index][ii][0];
			mean_y += Result_Dis_T_verify[cur_index][ii][1];
			num_xy++;
			if (Result_Dis_T_verify[cur_index][ii][0] > max_x)
			{
				max_x = Result_Dis_T_verify[cur_index][ii][0];
			}
			if (Result_Dis_T_verify[cur_index][ii][1] > max_y)
			{
				max_y = Result_Dis_T_verify[cur_index][ii][1];
			}
			if (Result_Dis_T_verify[cur_index][ii][0] < min_x)
			{
				min_x = Result_Dis_T_verify[cur_index][ii][0];
			}
			if (Result_Dis_T_verify[cur_index][ii][1] < min_y)
			{
				min_y = Result_Dis_T_verify[cur_index][ii][1];
			}
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_Dis_T_verify[cur_index][ii][0] - mean_x, 2);
			std_y += pow(Result_Dis_T_verify[cur_index][ii][1] - mean_y, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_K.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(Result_Dis_T[cur_index][0]);
			value_y_all.push_back(Result_Dis_T[cur_index][1]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_Dis_T[cur_index][0] > max_x)
			{
				max_x = Result_Dis_T[cur_index][0];
			}
			if (Result_Dis_T[cur_index][1] > max_y)
			{
				max_y = Result_Dis_T[cur_index][1];
			}
			if (Result_Dis_T[cur_index][0] < min_x)
			{
				min_x = Result_Dis_T[cur_index][0];
			}
			if (Result_Dis_T[cur_index][1] < min_y)
			{
				min_y = Result_Dis_T[cur_index][1];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);

		ui.show_stable_customplot->xAxis->setLabel("S1");
		ui.show_stable_customplot->yAxis->setLabel("S2");
		ui.show_stable_customplot->xAxis->setVisible(true);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(true);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}
		if (max_y != min_y)
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 0.1 * (max_y - min_y), min_y + 1.5 * (max_y - min_y));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 1e-4, min_y + 1e-4);
		}
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n S1: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / S2: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));

	}
	else if (ui.s3_s4_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, std_x = 0, std_y = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			value_x.push_back(Result_Dis_T_verify[cur_index][ii][2]);
			value_y.push_back(Result_Dis_T_verify[cur_index][ii][3]);
			mean_x += Result_Dis_T_verify[cur_index][ii][2];
			mean_y += Result_Dis_T_verify[cur_index][ii][3];
			num_xy++;
			if (Result_Dis_T_verify[cur_index][ii][2] > max_x)
			{
				max_x = Result_Dis_T_verify[cur_index][ii][2];
			}
			if (Result_Dis_T_verify[cur_index][ii][3] > max_y)
			{
				max_y = Result_Dis_T_verify[cur_index][ii][3];
			}
			if (Result_Dis_T_verify[cur_index][ii][2] < min_x)
			{
				min_x = Result_Dis_T_verify[cur_index][ii][2];
			}
			if (Result_Dis_T_verify[cur_index][ii][3] < min_y)
			{
				min_y = Result_Dis_T_verify[cur_index][ii][3];
			}
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_Dis_T_verify[cur_index][ii][2] - mean_x, 2);
			std_y += pow(Result_Dis_T_verify[cur_index][ii][3] - mean_y, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_K.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(Result_Dis_T[cur_index][2]);
			value_y_all.push_back(Result_Dis_T[cur_index][3]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_Dis_T[cur_index][2] > max_x)
			{
				max_x = Result_Dis_T[cur_index][2];
			}
			if (Result_Dis_T[cur_index][3] > max_y)
			{
				max_y = Result_Dis_T[cur_index][3];
			}
			if (Result_Dis_T[cur_index][2] < min_x)
			{
				min_x = Result_Dis_T[cur_index][2];
			}
			if (Result_Dis_T[cur_index][3] < min_y)
			{
				min_y = Result_Dis_T[cur_index][3];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setLabel("S3");
		ui.show_stable_customplot->yAxis->setLabel("S4");
		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);

		ui.show_stable_customplot->xAxis->setVisible(true);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(true);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->xAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}
		if (max_y != min_y)
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 0.1 * (max_y - min_y), min_y + 1.5 * (max_y - min_y));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_y - 1e-4, min_y + 1e-4);
		}
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foColumnsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n S3: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / S4: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));

	}
	else if (ui.k1_k2_k3_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(1);
		stable_sactter_view->setVisible(true);
		stable_sactter_view->axisX()->setTitle("K1");
		stable_sactter_view->axisX()->setTitleVisible(true);
		stable_sactter_view->axisY()->setTitle("K2");
		stable_sactter_view->axisY()->setTitleVisible(true);
		stable_sactter_view->axisZ()->setTitle("K3");
		stable_sactter_view->axisZ()->setTitleVisible(true);
		stable_sactter_view->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
		stable_sactter_view->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, mean_z = 0, std_x = 0, std_y = 0, std_z = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest(), max_z = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max(), min_z = std::numeric_limits<double>::max();
		
		for (int ii = 0; ii < Result_Dis_K_verify[cur_index].size(); ii++)
		{
			mean_x += Result_Dis_K_verify[cur_index][ii][0];
			mean_y += Result_Dis_K_verify[cur_index][ii][1];
			mean_z += Result_Dis_K_verify[cur_index][ii][2];
			num_xy++;
			if (Result_Dis_K_verify[cur_index][ii][0] > max_x)
			{
				max_x = Result_Dis_K_verify[cur_index][ii][0];
			}
			if (Result_Dis_K_verify[cur_index][ii][0] < min_x)
			{
				min_x = Result_Dis_K_verify[cur_index][ii][0];
			}
			if (Result_Dis_K_verify[cur_index][ii][1] > max_y)
			{
				max_y = Result_Dis_K_verify[cur_index][ii][1];
			}
			if (Result_Dis_K_verify[cur_index][ii][1] < min_y)
			{
				min_y = Result_Dis_K_verify[cur_index][ii][1];
			}
			if (Result_Dis_K_verify[cur_index][ii][2] > max_z)
			{
				max_z = Result_Dis_K_verify[cur_index][ii][2];
			}
			if (Result_Dis_K_verify[cur_index][ii][2] < min_z)
			{
				min_z = Result_Dis_K_verify[cur_index][ii][2];
			}
			*stable_sactter_dataArray << QVector3D(Result_Dis_K_verify[cur_index][ii][0], Result_Dis_K_verify[cur_index][ii][1], Result_Dis_K_verify[cur_index][ii][2]);
		}

		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		mean_z /= (double)num_xy;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_Dis_K_verify[cur_index][ii][0] - mean_x, 2);
			std_y += pow(Result_Dis_K_verify[cur_index][ii][1] - mean_y, 2);
			std_z += pow(Result_Dis_K_verify[cur_index][ii][2] - mean_z, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		std_z = sqrt(std_z / (double)num_xy);

		if (cur_index < Result_Dis_K.size())
		{
			if (Result_Dis_K[cur_index][0] > max_x)
			{
				max_x = Result_Dis_K[cur_index][0];
			}
			if (Result_Dis_K[cur_index][0] < min_x)
			{
				min_x = Result_Dis_K[cur_index][0];
			}
			if (Result_Dis_K[cur_index][1] > max_y)
			{
				max_y = Result_Dis_K[cur_index][1];
			}
			if (Result_Dis_K[cur_index][1] < min_y)
			{
				min_y = Result_Dis_K[cur_index][1];
			}
			if (Result_Dis_K[cur_index][2]  > max_z)
			{
				max_z = Result_Dis_K[cur_index][2];
			}
			if (Result_Dis_K[cur_index][2] < min_z)
			{
				min_z = Result_Dis_K[cur_index][2];
			}
			*stable_sactter_dataArray_real << QVector3D(Result_Dis_K[cur_index][0], Result_Dis_K[cur_index][1], Result_Dis_K[cur_index][2]);
			stable_sactter_series_real->setBaseColor(Qt::red);
			stable_sactter_proxy_real->resetArray(stable_sactter_dataArray_real);
		}
		ui.label_for_scatter_stable->setText(tr("(MIN - MAX - STD) \n K1: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / K2: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) +
			tr(") / K3: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
		stable_sactter_series->setBaseColor(Qt::blue);
		stable_sactter_proxy->resetArray(stable_sactter_dataArray);
		stable_sactter_view->setAspectRatio(1.0);
		stable_sactter_view->setHorizontalAspectRatio(1.0);
	}
	else if (ui.k4_k5_k6_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(1);
		stable_sactter_view->setVisible(true);
		stable_sactter_view->axisX()->setTitle("K4");
		stable_sactter_view->axisX()->setTitleVisible(true);
		stable_sactter_view->axisY()->setTitle("K5");
		stable_sactter_view->axisY()->setTitleVisible(true);
		stable_sactter_view->axisZ()->setTitle("K6");
		stable_sactter_view->axisZ()->setTitleVisible(true);
		stable_sactter_view->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
		stable_sactter_view->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, mean_z = 0, std_x = 0, std_y = 0, std_z = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest(), max_z = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max(), min_z = std::numeric_limits<double>::max();

		for (int ii = 0; ii < Result_Dis_K_verify[cur_index].size(); ii++)
		{
			mean_x += Result_Dis_K_verify[cur_index][ii][3];
			mean_y += Result_Dis_K_verify[cur_index][ii][4];
			mean_z += Result_Dis_K_verify[cur_index][ii][5];
			num_xy++;
			if (Result_Dis_K_verify[cur_index][ii][3] > max_x)
			{
				max_x = Result_Dis_K_verify[cur_index][ii][3];
			}
			if (Result_Dis_K_verify[cur_index][ii][3] < min_x)
			{
				min_x = Result_Dis_K_verify[cur_index][ii][3];
			}
			if (Result_Dis_K_verify[cur_index][ii][4] > max_y)
			{
				max_y = Result_Dis_K_verify[cur_index][ii][4];
			}
			if (Result_Dis_K_verify[cur_index][ii][4] < min_y)
			{
				min_y = Result_Dis_K_verify[cur_index][ii][4];
			}
			if (Result_Dis_K_verify[cur_index][ii][5] > max_z)
			{
				max_z = Result_Dis_K_verify[cur_index][ii][5];
			}
			if (Result_Dis_K_verify[cur_index][ii][5] < min_z)
			{
				min_z = Result_Dis_K_verify[cur_index][ii][5];
			}
			*stable_sactter_dataArray << QVector3D(Result_Dis_K_verify[cur_index][ii][3], Result_Dis_K_verify[cur_index][ii][4], Result_Dis_K_verify[cur_index][ii][5]);
		}
		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		mean_z /= (double)num_xy;
		for (int ii = 0; ii < Result_Dis_T_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_Dis_K_verify[cur_index][ii][3] - mean_x, 2);
			std_y += pow(Result_Dis_K_verify[cur_index][ii][4] - mean_y, 2);
			std_z += pow(Result_Dis_K_verify[cur_index][ii][5] - mean_z, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		std_z = sqrt(std_z / (double)num_xy);
		if (cur_index < Result_Dis_K.size())
		{
			if (Result_Dis_K[cur_index][3] > max_x)
			{
				max_x = Result_Dis_K[cur_index][3];
			}
			if (Result_Dis_K[cur_index][3] < min_x)
			{
				min_x = Result_Dis_K[cur_index][3];
			}
			if (Result_Dis_K[cur_index][4] > max_y)
			{
				max_y = Result_Dis_K[cur_index][4];
			}
			if (Result_Dis_K[cur_index][4] < min_y)
			{
				min_y = Result_Dis_K[cur_index][4];
			}
			if (Result_Dis_K[cur_index][5] > max_z)
			{
				max_z = Result_Dis_K[cur_index][5];
			}
			if (Result_Dis_K[cur_index][5] < min_z)
			{
				min_z = Result_Dis_K[cur_index][5];
			}
			*stable_sactter_dataArray_real << QVector3D(Result_Dis_K[cur_index][3], Result_Dis_K[cur_index][4], Result_Dis_K[cur_index][5]);
			stable_sactter_series_real->setBaseColor(Qt::red);
			stable_sactter_proxy_real->resetArray(stable_sactter_dataArray_real);
		}

		ui.label_for_scatter_stable->setText(tr("(MIN - MAX - STD) \n K4: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / K5: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) +
			tr(") / K6: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
		stable_sactter_series->setBaseColor(Qt::blue);
		stable_sactter_proxy->resetArray(stable_sactter_dataArray);
		stable_sactter_view->setAspectRatio(1.0);
		stable_sactter_view->setHorizontalAspectRatio(1.0);
	}
	else if (ui.Overall_R_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(1);
		stable_sactter_view->setVisible(true);
		stable_sactter_view->axisX()->setTitle("K1");
		stable_sactter_view->axisX()->setTitleVisible(true);
		stable_sactter_view->axisY()->setTitle("K2");
		stable_sactter_view->axisY()->setTitleVisible(true);
		stable_sactter_view->axisZ()->setTitle("K3");
		stable_sactter_view->axisZ()->setTitleVisible(true);
		stable_sactter_view->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
		stable_sactter_view->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, mean_z = 0, std_x = 0, std_y = 0, std_z = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest(), max_z = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max(), min_z = std::numeric_limits<double>::max();

		for (int ii = 0; ii < Result_R_overall_verify[cur_index].size(); ii++)
		{
			Eigen::Vector3d angle_temp = Result_R_overall_verify[cur_index][ii].toRotationMatrix().eulerAngles(0, 1, 2);
			mean_x += angle_temp[0];
			mean_y += angle_temp[1];
			mean_z += angle_temp[2];
			num_xy++;
			max_x = angle_temp[0] > max_x ? angle_temp[0] : max_x;
			max_y = angle_temp[1] > max_y ? angle_temp[1] : max_y;
			max_z = angle_temp[2] > max_z ? angle_temp[2] : max_z;
			min_x = angle_temp[0] < min_x ? angle_temp[0] : min_x;
			min_y = angle_temp[1] < min_y ? angle_temp[1] : min_y;
			min_z = angle_temp[2] < min_z ? angle_temp[2] : min_z;
			*stable_sactter_dataArray << QVector3D(angle_temp[0], angle_temp[1], angle_temp[2]);
		}

		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		mean_z /= (double)num_xy;
		for (int ii = 0; ii < Result_R_overall_verify[cur_index].size(); ii++)
		{
			Eigen::Vector3d angle_temp = Result_R_overall_verify[cur_index][ii].toRotationMatrix().eulerAngles(0, 1, 2);
			std_x += pow(angle_temp[0] - mean_x, 2);
			std_y += pow(angle_temp[1] - mean_y, 2);
			std_z += pow(angle_temp[2] - mean_z, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		std_z = sqrt(std_z / (double)num_xy);
		if (cur_index < Result_R_overall.size())
		{
			Eigen::Vector3d angle_temp = Result_R_overall[cur_index].toRotationMatrix().eulerAngles(0, 1, 2);
			max_x = angle_temp[0] > max_x ? angle_temp[0] : max_x;
			max_y = angle_temp[1] > max_y ? angle_temp[1] : max_y;
			max_z = angle_temp[2] > max_z ? angle_temp[2] : max_z;
			min_x = angle_temp[0] < min_x ? angle_temp[0] : min_x;
			min_y = angle_temp[1] < min_y ? angle_temp[1] : min_y;
			min_z = angle_temp[2] < min_z ? angle_temp[2] : min_z;
			*stable_sactter_dataArray_real << QVector3D(angle_temp[0], angle_temp[1], angle_temp[2]);
			stable_sactter_series_real->setBaseColor(Qt::red);
			stable_sactter_proxy_real->resetArray(stable_sactter_dataArray_real);
		}
		ui.label_for_scatter_stable->setText(tr("(MIN - MAX - STD) \n ThetaX: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / ThetaY: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) +
			tr(") / ThetaZ: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
		stable_sactter_series->setBaseColor(Qt::blue);
		stable_sactter_proxy->resetArray(stable_sactter_dataArray);
		stable_sactter_view->setAspectRatio(1.0);
		stable_sactter_view->setHorizontalAspectRatio(1.0);
	}
	else if (ui.Overall_T_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(1);
		stable_sactter_view->setVisible(true);
		stable_sactter_view->axisX()->setTitle("K1");
		stable_sactter_view->axisX()->setTitleVisible(true);
		stable_sactter_view->axisY()->setTitle("K2");
		stable_sactter_view->axisY()->setTitleVisible(true);
		stable_sactter_view->axisZ()->setTitle("K3");
		stable_sactter_view->axisZ()->setTitleVisible(true);
		stable_sactter_view->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
		stable_sactter_view->activeTheme()->setType(Q3DTheme::ThemePrimaryColors);
		int num_xy = 0;
		double mean_x = 0, mean_y = 0, mean_z = 0, std_x = 0, std_y = 0, std_z = 0;
		double max_x = std::numeric_limits<double>::lowest(), max_y = std::numeric_limits<double>::lowest(), max_z = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max(), min_y = std::numeric_limits<double>::max(), min_z = std::numeric_limits<double>::max();

		for (int ii = 0; ii < Result_T_overall_verify[cur_index].size(); ii++)
		{
			Eigen::Vector3d angle_temp = Result_T_overall_verify[cur_index][ii];
			mean_x += angle_temp[0];
			mean_y += angle_temp[1];
			mean_z += angle_temp[2];
			num_xy++;
			max_x = angle_temp[0] > max_x ? angle_temp[0] : max_x;
			max_y = angle_temp[1] > max_y ? angle_temp[1] : max_y;
			max_z = angle_temp[2] > max_z ? angle_temp[2] : max_z;
			min_x = angle_temp[0] < min_x ? angle_temp[0] : min_x;
			min_y = angle_temp[1] < min_y ? angle_temp[1] : min_y;
			min_z = angle_temp[2] < min_z ? angle_temp[2] : min_z;
			*stable_sactter_dataArray << QVector3D(angle_temp[0], angle_temp[1], angle_temp[2]);
		}

		mean_x /= (double)num_xy;
		mean_y /= (double)num_xy;
		mean_z /= (double)num_xy;
		for (int ii = 0; ii < Result_R_overall_verify[cur_index].size(); ii++)
		{
			Eigen::Vector3d angle_temp = Result_T_overall_verify[cur_index][ii];
			std_x += pow(angle_temp[0] - mean_x, 2);
			std_y += pow(angle_temp[1] - mean_y, 2);
			std_z += pow(angle_temp[2] - mean_z, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		std_y = sqrt(std_y / (double)num_xy);
		std_z = sqrt(std_z / (double)num_xy);

		if (cur_index < Result_T_overall.size())
		{
			Eigen::Vector3d angle_temp = Result_T_overall[cur_index];
			max_x = angle_temp[0] > max_x ? angle_temp[0] : max_x;
			max_y = angle_temp[1] > max_y ? angle_temp[1] : max_y;
			max_z = angle_temp[2] > max_z ? angle_temp[2] : max_z;
			min_x = angle_temp[0] < min_x ? angle_temp[0] : min_x;
			min_y = angle_temp[1] < min_y ? angle_temp[1] : min_y;
			min_z = angle_temp[2] < min_z ? angle_temp[2] : min_z;
			*stable_sactter_dataArray_real << QVector3D(angle_temp[0], angle_temp[1], angle_temp[2]);
			stable_sactter_series_real->setBaseColor(Qt::red);
			stable_sactter_proxy_real->resetArray(stable_sactter_dataArray_real);
		}
		ui.label_for_scatter_stable->setText(tr("(MIN - MAX - STD) \n TX: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(") / TY: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) +
			tr(") / TZ: (") + QString::number(min_y, 'f', 3) + tr(", ") + QString::number(max_y, 'f', 3)
			+ tr(", ") + QString::number(std_y, 'f', 3) + tr(")"));
		stable_sactter_series->setBaseColor(Qt::blue);
		stable_sactter_proxy->resetArray(stable_sactter_dataArray);
		stable_sactter_view->setAspectRatio(1.0);
		stable_sactter_view->setHorizontalAspectRatio(1.0);
	}
	else if (ui.shear_radioButton->isChecked())
	{
		ui.stackedWidget->setCurrentIndex(0);
		int num_xy = 0;
		double mean_x = 0, std_x = 0;
		double max_x = std::numeric_limits<double>::lowest()
			, min_x = std::numeric_limits<double>::max();
		QVector<double> value_x;
		QVector<double> value_y;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			value_x.push_back(1);
			value_y.push_back(Result_K_verify[cur_index][ii][4]);
			mean_x += Result_K_verify[cur_index][ii][4];
			num_xy++;
			if (Result_K_verify[cur_index][ii][4] > max_x)
			{
				max_x = Result_K_verify[cur_index][ii][4];
			}
			if (Result_K_verify[cur_index][ii][4] < min_x)
			{
				min_x = Result_K_verify[cur_index][ii][4];
			}
		}
		mean_x /= (double)num_xy;
		for (int ii = 0; ii < Result_K_verify[cur_index].size(); ii++)
		{
			std_x += pow(Result_K_verify[cur_index][ii][4] - mean_x, 2);
		}
		std_x = sqrt(std_x / (double)num_xy);
		ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
		QPen pen_scatter;
		pen_scatter.setWidth(3);
		pen_scatter.setColor(Qt::blue);
		pen_scatter.setStyle(Qt::PenStyle::SolidLine);
		ui.show_stable_customplot->graph(0)->setPen(pen_scatter);
		ui.show_stable_customplot->graph(0)->setLineStyle(QCPGraph::lsNone);
		ui.show_stable_customplot->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 5));
		ui.show_stable_customplot->graph(0)->setName(tr("Domain of solution"));
		ui.show_stable_customplot->graph(0)->setData(value_x, value_y);

		if (cur_index < Result_K.size())
		{
			ui.show_stable_customplot->addGraph(ui.show_stable_customplot->xAxis, ui.show_stable_customplot->yAxis);
			QVector<double> value_x_all;
			QVector<double> value_y_all;
			value_x_all.push_back(1);
			value_y_all.push_back(Result_K[cur_index][4]);
			QPen pen_scatter_real;
			pen_scatter_real.setWidth(3);
			pen_scatter_real.setColor(Qt::red);
			pen_scatter_real.setStyle(Qt::PenStyle::SolidLine);
			ui.show_stable_customplot->graph(1)->setPen(pen_scatter_real);
			ui.show_stable_customplot->graph(1)->setLineStyle(QCPGraph::lsNone);
			ui.show_stable_customplot->graph(1)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, 3));
			ui.show_stable_customplot->graph(1)->setName(tr("Real solution"));
			ui.show_stable_customplot->graph(1)->setData(value_x_all, value_y_all);

			if (Result_K[cur_index][4] > max_x)
			{
				max_x = Result_K[cur_index][4];
			}
			if (Result_K[cur_index][4] < min_x)
			{
				min_x = Result_K[cur_index][4];
			}
		}
		QSharedPointer<QCPAxisTicker> textTicker(new QCPAxisTicker);
		ui.show_stable_customplot->xAxis->setTicker(textTicker);
		QSharedPointer<QCPAxisTicker> textTicker2(new QCPAxisTicker);
		ui.show_stable_customplot->yAxis->setTicker(textTicker2);

		ui.show_stable_customplot->xAxis->setScaleType(QCPAxis::stLinear);
		ui.show_stable_customplot->yAxis->setScaleType(QCPAxis::stLinear);
		\
		ui.show_stable_customplot->xAxis->setLabel("");
		ui.show_stable_customplot->yAxis->setLabel("Skew");
		ui.show_stable_customplot->xAxis->setVisible(false);
		ui.show_stable_customplot->yAxis->setVisible(true);
		ui.show_stable_customplot->xAxis->setTickLabels(false);
		ui.show_stable_customplot->yAxis->setTickLabels(true);
		if (max_x != min_x)
		{
			ui.show_stable_customplot->yAxis->setRange(min_x - 0.1 * (max_x - min_x), max_x + 0.1 * (max_x - min_x));
		}
		else
		{
			ui.show_stable_customplot->yAxis->setRange(min_x - 1e-4, max_x + 1e-4);
		}

		ui.show_stable_customplot->xAxis->setRange(0.5, 1.5);
		ui.show_stable_customplot->legend->setFillOrder(QCPLayoutGrid::foRowsFirst);
		ui.show_stable_customplot->legend->setWrap(2);
		ui.show_stable_customplot->legend->setVisible(true);
		title_customplot_stable->setText(tr("(MIN - MAX - STD) \n Skew: (") + QString::number(min_x, 'f', 3) + tr(", ") + QString::number(max_x, 'f', 3)
			+ tr(", ") + QString::number(std_x, 'f', 3) +
			tr(")"));
	}
	ui.show_stable_customplot->replot(QCustomPlot::rpQueuedReplot);
}
void Camera_calibration_module::set_verify_showing_checked(bool enable)
{
	ui.fx_fy_radioButton->setChecked(enable);
	ui.cx_cy_radioButton->setChecked(enable);
	ui.k1_k2_k3_radioButton->setChecked(enable);
	ui.k4_k5_k6_radioButton->setChecked(enable);
	ui.p1_p2_radioButton->setChecked(enable);
	ui.s1_s2_radioButton->setChecked(enable);
	ui.s3_s4_radioButton->setChecked(enable);
	ui.Overall_R_radioButton->setChecked(enable);
	ui.Overall_T_radioButton->setChecked(enable);
	ui.shear_radioButton->setChecked(enable);
}

void Camera_calibration_module::set_verify_showing_enbale(bool enable)
{
	ui.fx_fy_radioButton->setEnabled(enable);
	ui.cx_cy_radioButton->setEnabled(enable);
	ui.k1_k2_k3_radioButton->setEnabled(enable);
	ui.k4_k5_k6_radioButton->setEnabled(enable);
	ui.p1_p2_radioButton->setEnabled(enable);
	ui.s1_s2_radioButton->setEnabled(enable);
	ui.s3_s4_radioButton->setEnabled(enable);
	ui.Overall_R_radioButton->setEnabled(enable);
	ui.Overall_T_radioButton->setEnabled(enable);
	ui.shear_radioButton->setEnabled(enable);
}

void Camera_calibration_module::show_Histogram_data(QMouseEvent* event)
{
	if (ui.show_plot_comboBox->currentIndex() == 0)
	{
		if (err_plot_showing.size() == 0)
		{
			return;
		}
		int x_pos = event->pos().x();
		int y_pos = event->pos().y();

		double x_val = ui.show_customplot->xAxis->pixelToCoord(x_pos);
		double y_val = ui.show_customplot->yAxis->pixelToCoord(y_pos);

		int x_i_value = qRound(x_val);

		int count = err_plot_showing[0].size();

		if (x_i_value > count || x_i_value < 1)
		{
			return;
		}
		if (y_val > cur_max_bar_showing)
		{
			return;
		}
		QString inf = tr("Image-") + QString::number(x_i_value) + "\n";
		for (int ii = 0; ii < err_plot_showing.size(); ii++)
		{
			inf = inf + tr("Camera-") + QString::number(err_plot_index_showing[ii] + 1) + tr(": ")
				+ QString::number(err_plot_showing[ii][x_i_value - 1], 'f', 3);
			if (ii != (err_plot_showing.size() - 1))
			{
				inf = inf + "\n";
			}
		}

		double x_tool_tip = ui.show_customplot->xAxis->coordToPixel(x_i_value);
		double y_tool_tip = ui.show_customplot->yAxis->coordToPixel(y_val);
		double xqcus_pos = ui.show_customplot->pos().x() + ui.plot_result_stackedWidget->pos().x() + ui.centralWidget->pos().x();
		double yqcus_pos = ui.show_customplot->pos().y() + ui.plot_result_stackedWidget->pos().y() + ui.centralWidget->pos().y();
		QToolTip::showText(mapToGlobal(QPoint(xqcus_pos + x_tool_tip, yqcus_pos + y_tool_tip)), inf, ui.show_customplot);
	}
}

void Camera_calibration_module::calibration_cameras()
{
	if (ui.target_comboBox->currentIndex() == 3 && (ui.chess_width_size_oricircle_spinBox->value() == 0 ||
		ui.offset_width_size_oricircle_spinBox->value() == 0 || ui.offset_height_size_oricircle_spinBox->value() == 0 || ui.circle_incre_width_size_oricircle_spinBox->value() == 0 ||
		ui.circle_incre_height_size_oricircle_spinBox->value() == 0 || ui.chess_height_size_oricircle_spinBox->value() == 0))
	{
		QMessageBox::warning(this,
			tr("Perform calibration"),
			tr("Error Calibration target parameters!"),
			QMessageBox::Ok,
			QMessageBox::Ok);
			return;
	}

	if (ui.target_comboBox->currentIndex() == 0)
	{
		cur_target_type_cal = Target_type::Chess_Board_type;
	}
	else if (ui.target_comboBox->currentIndex() == 1)
	{
		cur_target_type_cal = Target_type::Circle_Board_type;
	}
	else if (ui.target_comboBox->currentIndex() == 2)
	{
		cur_target_type_cal = Target_type::Ori_Circle_Board_type;
		cur_target_offset_for_ori_wid = ui.offset_width_size_oricircle_spinBox->value();
		cur_target_offset_for_ori_hei = ui.offset_height_size_oricircle_spinBox->value();
		cur_target_incre_for_ori_wid = ui.circle_incre_width_size_oricircle_spinBox->value();
		cur_target_incre_for_ori_hei = ui.circle_incre_height_size_oricircle_spinBox->value();
		cur_target_for_ori_wid = ui.chess_width_size_oricircle_spinBox->value();
		cur_target_for_ori_hei = ui.chess_height_size_oricircle_spinBox->value();
	}
	else if (ui.target_comboBox->currentIndex() == 3)
	{
		cur_target_type_cal = Target_type::Speckle_Board_type;
	}
	bool Image_group_valid[MAX_CALIBRAT_CAMERA];
	for (int jj = 0; jj < MAX_CALIBRAT_CAMERA; jj++)
	{
		Image_group_valid[jj] = false;
	}
	std::vector<std::vector< QString >> Image_group[MAX_CALIBRAT_CAMERA];
	std::vector<int> Image_group_to_index[MAX_CALIBRAT_CAMERA];
	int valid_calibrate_number = 0;
	for (int ii = 0; ii < ui.group_number_spinBox->value(); ii++)
	{
		for (int jj = 0; jj < MAX_CALIBRAT_CAMERA; jj++)
		{
			if (!Camera_box[jj]->isHidden())
			{
				if (Camera_combobox[jj]->currentText().toInt() == (ii+1))
				{
					Image_group[ii].push_back(Image_serial_name[jj]);
					Image_group_to_index[ii].push_back(jj);
				}
			}
		}
	}
	int number_canbe_cal = 0;
	QString label_inf = tr("Group Number:") + QString::number(ui.group_number_spinBox->value()) + "\n";
	for (int ii = 0; ii < ui.group_number_spinBox->value(); ii++)
	{
		if (Image_group[ii].size() != 0)
		{
			int gr_n = 0;
			for (int jj = 0; jj < Image_group[ii].size(); jj++)
			{
				if (Image_group[ii][jj].size() != 0)
				{
					gr_n = Image_group[ii][jj].size();
				}
			}
			if (gr_n != 0)
			{
				int valid_camera_number_in_group = 0;
				bool same_flag = true;
				for (int jj = 0; jj < Image_group[ii].size(); jj++)
				{
					if (Image_group[ii][jj].size() != 0)
					{
						if (Image_group[ii][jj].size() != gr_n)
						{
							same_flag = false;
							break;
						}
						else
						{
							valid_camera_number_in_group++;
						}
					}
				}
				if (same_flag)
				{
					if (gr_n >= 3)
					{
						label_inf = label_inf + tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
							+ tr(":") + QString("%1").arg(valid_camera_number_in_group, 2, 10, QLatin1Char('0')) + tr(" Cameras;")
							+ QString::number(gr_n)
							+ tr(" Images.") + "\n";
						number_canbe_cal++;
						Image_group_valid[ii] = true;
					}
					else
					{
						label_inf = label_inf + tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
							+ tr(":") + tr("Number of images less than 3 in camera-index.") + "\n";
					}
				}
				else
				{
					label_inf = label_inf + tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
						+ tr(":") + tr("Inconsistent number of images in camera-index.") + "\n";
				}
			}
			else
			{
				label_inf = label_inf + tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
					+ tr(":") + tr("Empty Data in camera-index.") + "\n";
			}
		}
		else
		{
			label_inf = label_inf + tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
				+ tr(":") + tr("Empty group.") + "\n";
		}
	}
	if (number_canbe_cal == 0)
	{
		QMessageBox::warning(this,
			tr("Perform calibration"),
			label_inf + tr("No group canbe calibrated!"),
			QMessageBox::Ok,
			QMessageBox::Ok);
		return;
	}
	QMessageBox::StandardButton re_inf=QMessageBox::information(this,
		tr("Perform calibration"),
		label_inf + tr("Execute calibration now?"),
		QMessageBox::Ok | QMessageBox::Cancel,
		QMessageBox::Ok);
	if (QMessageBox::Ok != re_inf)
	{
		return;
	}
	if (Result_K.size() != 0)
	{
		for (int ii = 0; ii < Result_K.size(); ii++)
		{
			delete[]Result_K[ii];
		}
	}
	if (Result_Dis_K.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K.size(); ii++)
		{
			delete[]Result_Dis_K[ii];
		}
	}
	if (Result_Dis_P.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P.size(); ii++)
		{
			delete[]Result_Dis_P[ii];
		}
	}
	if (Result_Dis_T.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T.size(); ii++)
		{
			delete[]Result_Dis_T[ii];
		}
	}
	Result_costdata_single.clear();
	Result_costdata_all.clear();
	Result_K.clear();
	Result_Dis_K.clear();
	Result_Dis_P.clear();
	Result_Dis_T.clear();
	Result_R.clear();
	Result_R_overall.clear();
	Result_T_overall.clear();
	Result_Re_map.clear();
	Result_re_err.clear();
	Result_update_calib_points.clear();
	Result_isenable.clear();
	Result_index.clear();
	if (Result_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_K_verify[ii].size(); jj++)
			{
				delete[] Result_K_verify[ii][jj];
			}
		}
	}
	Result_K_verify.clear();
	if (Result_Dis_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_K_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_K_verify[ii][jj];
			}
		}
	}
	Result_Dis_K_verify.clear();
	if (Result_Dis_P_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_P_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_P_verify[ii][jj];
			}
		}
	}
	Result_Dis_P_verify.clear();
	if (Result_Dis_T_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_T_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_T_verify[ii][jj];
			}
		}
	}
	Result_Dis_T_verify.clear();
	Result_R_overall_verify.clear();
	Result_T_overall_verify.clear();
	Result_update_calib_points_verify.clear();
	QProgressDialog oQProgressDialog;
	oQProgressDialog.setWindowModality(Qt::ApplicationModal);
	oQProgressDialog.setWindowTitle(tr("Detect feature points..."));
	oQProgressDialog.setRange(0, 1);
	oQProgressDialog.setMinimumDuration(0);
	
	for (int ii = 0; ii < ui.group_number_spinBox->value(); ii++)
	{
		if (!Image_group_valid[ii])
		{
			continue;
		}
		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			if (!KeyPointNeedRecal_serial[Image_group_to_index[ii][jj]])
			{
				continue;
			}

			KeyPoint_serial[Image_group_to_index[ii][jj]].clear();
			Result_contours_serial[Image_group_to_index[ii][jj]].clear();
			KeyPointWidth_serial[Image_group_to_index[ii][jj]].clear();
			KeyPointHeight_serial[Image_group_to_index[ii][jj]].clear();
			ImageWidth_serial[Image_group_to_index[ii][jj]].clear();
			ImageHeight_serial[Image_group_to_index[ii][jj]].clear();

			oQProgressDialog.setLabelText(tr("Detect for Camera-") + QString("%1").arg(Image_group_to_index[ii][jj] + 1, 2, 10, QLatin1Char('0'))
			+tr(":") + QString::number(0) + tr("/") + QString::number(Image_group[ii][jj].size()));
			oQProgressDialog.setRange(0, Image_group[ii][jj].size());
			oQProgressDialog.setValue(0);
			qApp->processEvents();
			for (int kk = 0; kk < Image_group[ii][jj].size(); kk++)
			{
				cv::Mat img_temp;
				cv::Mat gray_temp;
				cv::Size checkerboard_size;
				std::vector<cv::Point2f> corner_points_cur = {};
				std::vector<std::vector<cv::Point2f>> edge_points_cur;
				oQProgressDialog.setValue((kk - 1) < 0 ? 0 : (kk - 1));
				cv::Mat img_cur = cv::imread(Image_group[ii][jj][kk].toLocal8Bit().toStdString());
				if (img_cur.empty())
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].push_back(corner_points_cur);
					Result_contours_serial[Image_group_to_index[ii][jj]].push_back(edge_points_cur);
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].push_back(0);
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].push_back(0);
					ImageWidth_serial[Image_group_to_index[ii][jj]].push_back(0);
					ImageHeight_serial[Image_group_to_index[ii][jj]].push_back(0);
				}
				img_temp = img_cur;
				cv::cvtColor(img_temp, gray_temp, CV_BGR2GRAY);
				oQProgressDialog.setValue((kk - 1) < 0 ? 0 : (kk - 1));

				qApp->processEvents();
				if (oQProgressDialog.wasCanceled())
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].clear();
					Result_contours_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].clear();
					ImageWidth_serial[Image_group_to_index[ii][jj]].clear();
					ImageHeight_serial[Image_group_to_index[ii][jj]].clear();
					QMessageBox::information(this, tr("Process"), tr("User Stop"), QMessageBox::Ok);
					return;
				}
				bool found_fin = false;
				checkerboard_size.width = 0;
				checkerboard_size.height = 0;
				if (ui.target_comboBox->currentIndex() == 0)
				{
					found_fin = ImageDetectMethod::detectCheckerboard(gray_temp, corner_points_cur, checkerboard_size,
						cam_model_ui->ui.peak_threashold_doubleSpinBox->value(), cam_model_ui->ui.high_distortion_checkBox->isChecked());
				}
				else if(ui.target_comboBox->currentIndex() == 2)
				{
					std::vector<uchar> list;
					int useful_corner_num = 0;
					int circle_wid = ui.chess_width_size_oricircle_spinBox->value();
					int circle_hei = ui.chess_height_size_oricircle_spinBox->value();
					int offset_wid = ui.offset_width_size_oricircle_spinBox->value();
					int offset_hei = ui.offset_height_size_oricircle_spinBox->value();
					int incre_wid = ui.circle_incre_width_size_oricircle_spinBox->value();
					int incre_hei = ui.circle_incre_height_size_oricircle_spinBox->value();
					found_fin = ImageDetectMethod::FindCircleGrid(gray_temp, circle_wid, circle_hei,
						offset_wid, offset_hei, incre_wid, incre_hei,
						corner_points_cur, list, useful_corner_num, edge_points_cur, ui.max_ratio_spinBox->value(), 1.0 / ui.radius_k_doubleSpinBox->value()
						, ui.min_R_spinBox->value(), ui.max_R_spinBox->value(), ui.max_error_doubleSpinBox->value(), ui.max_arc_spinBox->value(),
						ui.max_P_spinBox->value(), ui.max_KP_spinBox->value());

					
					if (found_fin)
					{
						double r_first = sqrt(pow(corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid - 1].x - 
							corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid + incre_wid - 1].x, 2) 
							+ pow(corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid - 1].y - 
								corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid + incre_wid - 1].y, 2)) / (double)incre_wid * 0.5
							+ sqrt(pow(corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid - 1].x -
								corner_points_cur[(offset_hei + incre_hei - 1) * circle_wid + offset_wid - 1].x, 2)
								+ pow(corner_points_cur[(offset_hei - 1) * circle_wid + offset_wid - 1].y -
									corner_points_cur[(offset_hei + incre_hei - 1) * circle_wid + offset_wid - 1].y, 2)) / (double)incre_hei * 0.5;
						int inter_c = (int)((double)gray_temp.cols / (double)(circle_wid + 1)) > (int)((double)gray_temp.rows / (double)(circle_hei + 1))
							? (int)((double)gray_temp.cols / (double)(circle_wid + 1)) : (int)((double)gray_temp.rows / (double)(circle_hei + 1));
						inter_c = inter_c > (int)((double)gray_temp.rows / (double)(circle_wid + 1))
							? inter_c : (int)((double)gray_temp.rows / (double)(circle_wid + 1));
						inter_c = inter_c > (int)((double)gray_temp.cols / (double)(circle_hei + 1))
							? inter_c : (int)((double)gray_temp.cols / (double)(circle_hei + 1));
						double r_multi = (double)inter_c / (double)r_first;

						std::vector<cv::Point2f> AffinePointsSrc;
						std::vector<cv::Point2f> AffinePointsDst;
						int valid_pp = 0;
						for (int pp_nums = 0; pp_nums < circle_hei; pp_nums++)
						{
							for (int qq_nums = 0; qq_nums < circle_wid; qq_nums++)
							{
								if (corner_points_cur[pp_nums * circle_wid + qq_nums].x != 0 && corner_points_cur[pp_nums * circle_wid + qq_nums].y != 0)
								{
									AffinePointsSrc.push_back(corner_points_cur[pp_nums * circle_wid + qq_nums]);
									AffinePointsDst.push_back(cv::Point2f((float)inter_c* (float)(pp_nums + 1), (float)inter_c* (float)(qq_nums + 1)));
									valid_pp++;
								}
							}
						}
						if (!(valid_pp < (circle_hei * circle_wid) * 0.5 || valid_pp < 8))
						{
							cv::Mat per_tf = cv::findHomography(AffinePointsSrc, AffinePointsDst, cv::RANSAC);

							for (unsigned int gg = 0; gg < corner_points_cur.size(); gg++)
							{
								if (corner_points_cur[gg].x == 0 && corner_points_cur[gg].y == 0)
								{
									continue;
								}

								std::vector<cv::Point2f> con_new;
								for (int tt = 0; tt < edge_points_cur[gg].size(); tt++)
								{
									cv::Mat_<double> mat_pt(3, 1);
									mat_pt(0, 0) = edge_points_cur[gg][tt].x;
									mat_pt(1, 0) = edge_points_cur[gg][tt].y;
									mat_pt(2, 0) = 1;
									cv::Mat mat_pt_view = per_tf * mat_pt;
									double a1 = mat_pt_view.at<double>(0, 0);
									double a2 = mat_pt_view.at<double>(1, 0);
									double a3 = mat_pt_view.at<double>(2, 0);
									con_new.push_back(cv::Point2f(a1 / a3, a2 / a3));
								}
								cv::RotatedRect rRect = fitEllipseAMS(con_new);
								if (isnan(rRect.center.x) || isinf(rRect.center.x))
								{
									continue;
								}
								if (isnan(rRect.center.y) || isinf(rRect.center.y))
								{
									continue;
								}
								if (isnan(rRect.size.width) || isinf(rRect.size.width))
								{
									continue;
								}
								if (isnan(rRect.size.height) || isinf(rRect.size.height))
								{
									continue;
								}
								cv::Mat_<double> mat_pt_inv(3, 1);
								mat_pt_inv(0, 0) = rRect.center.x;
								mat_pt_inv(1, 0) = rRect.center.y;
								mat_pt_inv(2, 0) = 1;
								cv::Mat mat_pt_view = per_tf.inv() * mat_pt_inv;
								double a1 = mat_pt_view.at<double>(0, 0);
								double a2 = mat_pt_view.at<double>(1, 0);
								double a3 = mat_pt_view.at<double>(2, 0);
								corner_points_cur[gg].x = a1 / a3;
								corner_points_cur[gg].y = a2 / a3;
							}
						}
						checkerboard_size.width = circle_wid + 1;
						checkerboard_size.height = circle_hei + 1;
					}
					else
					{
						corner_points_cur.clear();
					}
				}

				qApp->processEvents();
				if (oQProgressDialog.wasCanceled())
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].clear();
					Result_contours_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].clear();
					ImageWidth_serial[Image_group_to_index[ii][jj]].clear();
					ImageHeight_serial[Image_group_to_index[ii][jj]].clear();
					QMessageBox::information(this, tr("Process"), tr("User Stop"), QMessageBox::Ok);
					return;
				}
				if (found_fin && corner_points_cur.size() > 2)
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].push_back(corner_points_cur);
					Result_contours_serial[Image_group_to_index[ii][jj]].push_back(edge_points_cur);
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].push_back(checkerboard_size.width - 1);
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].push_back(checkerboard_size.height - 1);
					ImageWidth_serial[Image_group_to_index[ii][jj]].push_back(img_temp.cols);
					ImageHeight_serial[Image_group_to_index[ii][jj]].push_back(img_temp.rows);
				}
				else
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].push_back(corner_points_cur);
					Result_contours_serial[Image_group_to_index[ii][jj]].push_back(edge_points_cur);
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].push_back(0);
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].push_back(0);
					ImageWidth_serial[Image_group_to_index[ii][jj]].push_back(img_temp.cols);
					ImageHeight_serial[Image_group_to_index[ii][jj]].push_back(img_temp.rows);
				}
				oQProgressDialog.setLabelText(tr("Detect for Camera-") + QString("%1").arg(Image_group_to_index[ii][jj] + 1, 2, 10, QLatin1Char('0'))
					+ tr(":") + QString::number(kk) + tr("/") + QString::number(Image_group[ii][jj].size()));
				oQProgressDialog.setValue(kk);
				ui.curren_index_spinBox->setValue(Image_group_to_index[ii][jj] + 1);
				ui.show_detect_horizontalSlider->setValue(kk + 1);
				ui.show_detect_spinBox->setValue(kk + 1);
				qApp->processEvents();
				if (oQProgressDialog.wasCanceled())
				{
					KeyPoint_serial[Image_group_to_index[ii][jj]].clear();
					Result_contours_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointWidth_serial[Image_group_to_index[ii][jj]].clear();
					KeyPointHeight_serial[Image_group_to_index[ii][jj]].clear();
					ImageWidth_serial[Image_group_to_index[ii][jj]].clear();
					ImageHeight_serial[Image_group_to_index[ii][jj]].clear();
					QMessageBox::information(this, tr("Process"), tr("User Stop"), QMessageBox::Ok);
					return;
				}
			}
			KeyPointNeedRecal_serial[Image_group_to_index[ii][jj]] = false;

		}
	}

	for (int ii = 0; ii < ui.group_number_spinBox->value(); ii++)
	{
		if (!Image_group_valid[ii])
		{
			continue;
		}
		std::vector<std::vector<std::vector<cv::Point2f>>> point_serial;
		std::vector <std::vector<int>> many_width;
		std::vector<std::vector<int>> many_height;
		std::vector < std::vector<int>> many_number;
		std::vector<int> valid_width;
		std::vector<int> valid_height;

		std::vector<int> remove_index;

		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			many_width.push_back({});
			many_height.push_back({});
			many_number.push_back({});
			valid_width.push_back(-1);
			valid_height.push_back(-1);
		}
		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			for (int kk = 0; kk < Image_group[ii][jj].size(); kk++)
			{
				if (many_width[jj].size() == 0)
				{
					many_width[jj].push_back(ImageWidth_serial[Image_group_to_index[ii][jj]][kk]);
					many_height[jj].push_back(ImageHeight_serial[Image_group_to_index[ii][jj]][kk]);
					many_number[jj].push_back(1);
				}
				else
				{
					bool is_exist = false;
					for (int pp = 0; pp < many_width[jj].size(); pp++)
					{
						if (ImageWidth_serial[Image_group_to_index[ii][jj]][kk] == many_width[jj][pp]
							&& ImageHeight_serial[Image_group_to_index[ii][jj]][kk] == many_height[jj][pp])
						{
							is_exist = true;
							many_number[jj][pp]++;
							break;
						}
					}
					if (!is_exist)
					{
						many_width[jj].push_back(ImageWidth_serial[Image_group_to_index[ii][jj]][kk]);
						many_height[jj].push_back(ImageHeight_serial[Image_group_to_index[ii][jj]][kk]);
						many_number[jj].push_back(1);
					}
				}
			}
		}
		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			int max_number;
			for (int kk = 0; kk < many_number[jj].size(); kk++)
			{
				if (kk == 0)
				{
					max_number = many_number[jj][kk];
					valid_width[jj] = many_width[jj][kk];
					valid_height[jj] = many_height[jj][kk];
				}
				else
				{
					if (many_number[jj][kk] > max_number && many_width[jj][kk] != 0 && many_height[jj][kk] != 0)
					{
						max_number = many_number[jj][kk];
						valid_width[jj] = many_width[jj][kk];
						valid_height[jj] = many_height[jj][kk];
					}
				}
			}
		}


		std::vector<int> many_width_size;
		std::vector<int> many_height_size;
		std::vector<int> many_number_size;
		int valid_width_size;
		int valid_height_size;
		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			for (int kk = 0; kk < Image_group[ii][jj].size(); kk++)
			{
				if (many_width_size.size() == 0)
				{
					many_width_size.push_back(KeyPointWidth_serial[Image_group_to_index[ii][jj]][kk]);
					many_height_size.push_back(KeyPointHeight_serial[Image_group_to_index[ii][jj]][kk]);
					many_number_size.push_back(1);
				}
				else
				{
					bool is_exist = false;
					for (int pp = 0; pp < many_width_size.size(); pp++)
					{
						if (KeyPointWidth_serial[Image_group_to_index[ii][jj]][kk] == many_width_size[pp]
							&& KeyPointHeight_serial[Image_group_to_index[ii][jj]][kk] == many_height_size[pp])
						{
							is_exist = true;
							many_number_size[pp]++;
							break;
						}
					}
					if (!is_exist)
					{
						many_width_size.push_back(KeyPointWidth_serial[Image_group_to_index[ii][jj]][kk]);
						many_height_size.push_back(KeyPointHeight_serial[Image_group_to_index[ii][jj]][kk]);
						many_number_size.push_back(1);
					}
				}
			}
		}
		int max_number_size;
		for (int kk = 0; kk < many_number_size.size(); kk++)
		{
			if (kk == 0)
			{
				max_number_size = many_number_size[kk];
				valid_width_size = many_width_size[kk];
				valid_height_size = many_height_size[kk];
			}
			else
			{
				if (many_number_size[kk] > max_number_size && many_width_size[kk]!=0 && many_height_size[kk] != 0)
				{
					max_number_size = many_number_size[kk];
					valid_width_size = many_width_size[kk];
					valid_height_size = many_height_size[kk];
				}
			}
		}
		std::vector<std::vector<std::vector<cv::Point3f>>> obj_pt_vec;
		std::vector<std::vector<std::vector<cv::Point2f>>> img_pt_vec;
		std::vector<std::vector<std::vector<cv::Point2f>>> board_pt_vec;

		for (int jj = 0; jj < Image_group[ii].size(); jj++)
		{
			std::vector<std::vector<cv::Point3f>> obj_pt;
			std::vector<std::vector<cv::Point2f>> img_pt;
			std::vector<std::vector<cv::Point2f>> board_pt;
			if (Image_group[ii][jj].size() == 0)
			{
				continue;
			}
			for (int kk = 0; kk < Image_group[ii][jj].size(); kk++)
			{
				if (KeyPointWidth_serial[Image_group_to_index[ii][jj]][kk] == valid_width_size &&
					KeyPointHeight_serial[Image_group_to_index[ii][jj]][kk] == valid_height_size &&
					ImageWidth_serial[Image_group_to_index[ii][jj]][kk] == valid_width[jj] &&
					ImageHeight_serial[Image_group_to_index[ii][jj]][kk] == valid_height[jj])
				{
					std::vector<cv::Point3f> obj_pt_temp;
					std::vector<cv::Point2f> img_pt_temp;
					std::vector<cv::Point2f> board_pt_temp;
					for (int pp = 0; pp < KeyPoint_serial[Image_group_to_index[ii][jj]][kk].size(); pp++)
					{
						if (ui.target_comboBox->currentIndex() == 0)
						{
							obj_pt_temp.push_back(cv::Point3f((double)(pp / valid_width_size) * ui.chess_height_length_doubleSpinBox->value(),
								(double)(pp % valid_width_size) * ui.chess_width_length_doubleSpinBox->value(), 0));
							board_pt_temp.push_back(cv::Point2f((double)(pp / valid_width_size) * ui.chess_height_length_doubleSpinBox->value(),
								(double)(pp % valid_width_size) * ui.chess_width_length_doubleSpinBox->value()));
							img_pt_temp.push_back(KeyPoint_serial[Image_group_to_index[ii][jj]][kk][pp]);
						}
						else if (ui.target_comboBox->currentIndex() == 2)
						{
							obj_pt_temp.push_back(cv::Point3f((double)(pp / valid_width_size) * ui.chess_height_length_oricircle_doubleSpinBox->value(),
								(double)(pp % valid_width_size) * ui.chess_width_length_oricircle_doubleSpinBox->value(), 0));
							board_pt_temp.push_back(cv::Point2f((double)(pp / valid_width_size) * ui.chess_height_length_oricircle_doubleSpinBox->value(),
								(double)(pp % valid_width_size) * ui.chess_width_length_oricircle_doubleSpinBox->value()));
							img_pt_temp.push_back(KeyPoint_serial[Image_group_to_index[ii][jj]][kk][pp]);
						}
					}
					obj_pt.push_back(obj_pt_temp);
					img_pt.push_back(img_pt_temp);
					board_pt.push_back(board_pt_temp);
				}
				else
				{
					obj_pt.push_back({});
					img_pt.push_back({});
					board_pt.push_back({});
				}
				if (!KeyPoint_Enable_serial[Image_group_to_index[ii][jj]][kk])
				{
					bool has_exis = false;
					for (int ss = 0; ss < remove_index.size(); ss++)
					{
						if (kk == remove_index[ss])
						{
							has_exis = true;
							break;
						}
					}
					if (!has_exis)
					{
						remove_index.push_back(kk);
					}
				}
			}
			obj_pt_vec.push_back(obj_pt);
			img_pt_vec.push_back(img_pt);
			board_pt_vec.push_back(board_pt);
			Result_index.push_back(Image_group_to_index[ii][jj]);
		}

		for (int pp = 0; pp < obj_pt_vec.size(); pp++)
		{
			for (int qq = 0; qq < obj_pt_vec[pp].size(); qq++)
			{
				if (obj_pt_vec[pp][qq].size() == 0)
				{
					bool has_exis = false;
					for (int kk = 0; kk < remove_index.size(); kk++)
					{
						if (qq == remove_index[kk])
						{
							has_exis = true;
							break;
						}
					}
					if (!has_exis)
					{
						remove_index.push_back(qq);
					}
				}
			}
		}
		for (int pp = 0; pp < obj_pt_vec.size(); pp++)
		{
			std::vector<bool> enable_flag;
			for (int qq = 0; qq < obj_pt_vec[pp].size(); qq++)
			{
				bool has_exis = false;
				for (int kk = 0; kk < remove_index.size(); kk++)
				{
					if (qq == remove_index[kk])
					{
						has_exis = true;
						break;
					}
				}
				if (!has_exis)
				{
					enable_flag.push_back(true);
				}
				else
				{
					enable_flag.push_back(false);
				}
			}
			Result_isenable.push_back(enable_flag);
		}

		std::sort(remove_index.begin(), remove_index.end());
		for (int pp = remove_index.size() - 1; pp >= 0; pp--)
		{
			for (int jj = 0; jj < obj_pt_vec.size(); jj++)
			{
				obj_pt_vec[jj].erase(obj_pt_vec[jj].begin() + remove_index[pp]);
				img_pt_vec[jj].erase(img_pt_vec[jj].begin() + remove_index[pp]);
				board_pt_vec[jj].erase(board_pt_vec[jj].begin() + remove_index[pp]);
			}
		}

		if (obj_pt_vec.size() < 1)
		{
			QMessageBox::warning(this, tr("Detect Error"), tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
				+ tr(":") + tr("No camereas!"), QMessageBox::Ok, QMessageBox::Ok);
			continue;
		}
		if (obj_pt_vec[0].size() < 3)
		{
			QMessageBox::warning(this, tr("Detect Error"), tr("Group-") + QString("%1").arg(ii + 1, 2, 10, QLatin1Char('0'))
				+ tr(":") + tr("Number of images canbe detected less than 3 in camera-index!"), QMessageBox::Ok, QMessageBox::Ok);
			continue;
		}

		cam_log_ui->show();
		cam_log_ui->ui.textEdit->clear();
		qApp->processEvents();
		oQProgressDialog.setWindowTitle(tr("Optimize param..."));
		oQProgressDialog.setRange(0, obj_pt_vec.size() + 2);
		oQProgressDialog.setValue(1);
		oQProgressDialog.reset();
		std::vector<int> fix_index;
		std::vector<int> zero_index;
		if (cur_target_type_cal == Target_type::Chess_Board_type)
		{
			fix_index.push_back(valid_width_size* valid_height_size - valid_width_size);
			fix_index.push_back(valid_width_size - 1);
			zero_index.push_back(0);
		}
		else
		{
			fix_index.push_back((cur_target_offset_for_ori_hei - 1 + cur_target_incre_for_ori_hei)* cur_target_for_ori_wid
				+ cur_target_offset_for_ori_wid - 1);
			fix_index.push_back((cur_target_offset_for_ori_hei - 1)* cur_target_for_ori_wid
				+ cur_target_offset_for_ori_wid + cur_target_incre_for_ori_wid - 1);
			zero_index.push_back((cur_target_offset_for_ori_hei - 1)* cur_target_for_ori_wid
				+ cur_target_offset_for_ori_wid - 1);
		}
		std::vector<double*> K_vec;
		std::vector<double*> Dis_K_vec;
		std::vector<double*> Dis_P_vec;
		std::vector<double*> Dis_T_vec;
		std::vector<std::vector<Eigen::Quaterniond>> R_vec;
		std::vector<std::vector<Eigen::Vector3d>> T_vec;
		std::vector<std::vector<std::vector<cv::Point2f>>> Re_map_vec;
		std::vector < std::vector<cv::Point3f>> update_plate_vec;
		std::vector<std::vector<double>> re_err_vec;
		for (int kk = 0; kk < obj_pt_vec.size(); kk++)
		{
			qApp->processEvents();
			if (oQProgressDialog.wasCanceled())
			{
				QMessageBox::information(this, tr("Process"), tr("User Stop"), QMessageBox::Ok);
				return;
			}
			oQProgressDialog.setLabelText(tr("Optimize param for Index-") + QString("%1").arg(kk, 2, 10, QLatin1Char('0')));
			oQProgressDialog.setValue(kk + 1);
			qApp->processEvents();
			double* K = nullptr;
			double* Dis_K = nullptr;
			double* Dis_P = nullptr;
			double* Dis_T = nullptr;
			std::vector<Eigen::Quaterniond> R;
			std::vector<Eigen::Vector3d> T;
			std::vector<std::vector<cv::Point2f>> Re_map;
			std::vector<double> re_err;

			cam_log_ui->ui.textEdit->clear();
			ceres::Solver::Summary summary;
			if (cam_model_ui->ui.optimize_checkBox->isChecked())
			{
				calculate_camera_parame_fixed_optimize_thread thread_cal(valid_width_size* valid_height_size,
					valid_width[kk], valid_height[kk],
					fix_index, zero_index,
					img_pt_vec[kk], board_pt_vec[kk], obj_pt_vec[kk][1], &summary
					, cam_model_ui->ui.max_iter_spinBox->value()
					, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
					, cam_model_ui->ui.thread_spinBox->value()
					, cam_model_ui->ui.timeout_spinBox->value()
					, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
					, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.skew_checkBox->isChecked()
					, cam_model_ui->ui.uniform_f_checkBox->isChecked());
				thread_cal.start();

				while (1)
				{
					if (!thread_cal.isRunning())
					{
						break;
					}
					QString inf_log = "";
					for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
					{
						inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
						inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
						inf_log = inf_log + tr("\n");
					}
					cam_log_ui->ui.textEdit->setText(inf_log);
					loop_sleep(20);
				}
				std::vector<double> cost_plot;
				for (int ss = 0; ss < summary.iterations.size(); ss++)
				{
					cost_plot.push_back(summary.iterations[ss].cost);
				}
				Result_costdata_single.push_back(cost_plot);
				Result_costdata_all.push_back(cost_plot);
				K = thread_cal.K;
				Dis_K = thread_cal.Dis_K;
				Dis_P = thread_cal.Dis_P;
				Dis_T = thread_cal.Dis_T;
				R = thread_cal.R;
				T = thread_cal.T;
				Re_map = thread_cal.Re_map;
				re_err = thread_cal.re_err;
				obj_pt_vec[kk][0] = thread_cal.obj_pt;

				K_vec.push_back(K);
				Dis_K_vec.push_back(Dis_K);
				Dis_P_vec.push_back(Dis_P);
				Dis_T_vec.push_back(Dis_T);
				R_vec.push_back(R);
				T_vec.push_back(T);
				Re_map_vec.push_back(Re_map);
				re_err_vec.push_back(re_err);
				update_plate_vec.push_back(obj_pt_vec[kk][0]);
			}
			else
			{
				calculate_camera_parame_thread thread_cal(valid_width[kk], valid_height[kk],
					img_pt_vec[kk], board_pt_vec[kk], obj_pt_vec[kk], &summary
					, cam_model_ui->ui.max_iter_spinBox->value()
					, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
					, cam_model_ui->ui.thread_spinBox->value()
					, cam_model_ui->ui.timeout_spinBox->value()
					, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
					, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.skew_checkBox->isChecked()
					, cam_model_ui->ui.uniform_f_checkBox->isChecked());
				thread_cal.start();

				while (1)
				{
					if (!thread_cal.isRunning())
					{
						break;
					}
					QString inf_log = "";
					for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
					{
						inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
						inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
						inf_log = inf_log + tr("\n");
					}
					cam_log_ui->ui.textEdit->setText(inf_log);
					loop_sleep(20);
				}
				std::vector<double> cost_plot;
				for (int ss = 0; ss < summary.iterations.size(); ss++)
				{
					cost_plot.push_back(summary.iterations[ss].cost);
				}
				Result_costdata_single.push_back(cost_plot);
				Result_costdata_all.push_back(cost_plot);
				K = thread_cal.K;
				Dis_K = thread_cal.Dis_K;
				Dis_P = thread_cal.Dis_P;
				Dis_T = thread_cal.Dis_T;
				R = thread_cal.R;
				T = thread_cal.T;
				Re_map = thread_cal.Re_map;
				re_err = thread_cal.re_err;
				obj_pt_vec[kk][0] = thread_cal.obj_pt[0];

				K_vec.push_back(K);
				Dis_K_vec.push_back(Dis_K);
				Dis_P_vec.push_back(Dis_P);
				Dis_T_vec.push_back(Dis_T);
				R_vec.push_back(R);
				T_vec.push_back(T);
				Re_map_vec.push_back(Re_map);
				re_err_vec.push_back(re_err);
				update_plate_vec.push_back(obj_pt_vec[kk][0]);
			}
		}
		if (obj_pt_vec.size() == 1)
		{
			oQProgressDialog.setValue(obj_pt_vec.size() + 1);
			qApp->processEvents();
			Eigen::Matrix3d R_now;
			R_now << 1, 0, 0, 0, 1, 0, 0, 0, 1;
			Eigen::Vector3d T_now;
			T_now << 0, 0, 0;
			Result_K.push_back(K_vec[0]);
			Result_Dis_K.push_back(Dis_K_vec[0]);
			Result_Dis_P.push_back(Dis_P_vec[0]);
			Result_Dis_T.push_back(Dis_T_vec[0]);
			Result_R.push_back(R_vec[0]);
			Result_T.push_back(T_vec[0]);
			Result_R_overall.push_back(Eigen::Quaterniond(R_now));
			Result_R_overall[Result_R_overall.size() - 1].normalize();
			Result_T_overall.push_back(T_now);
			Result_Re_map.push_back(Re_map_vec[0]);
			Result_re_err.push_back(re_err_vec[0]);
			Result_costdata_all.at(Result_costdata_all.size() - 1).clear();
			for (int ss = 0; ss < Result_isenable.at(Result_isenable.size() - 1).size(); ss++)
			{
				if (!Result_isenable.at(Result_isenable.size() - 1)[ss])
				{
					Result_T.at(Result_T.size() - 1).insert(Result_T.at(Result_T.size() - 1).begin() + ss, Eigen::Vector3d(0, 0, 0));
					Result_R.at(Result_R.size() - 1).insert(Result_R.at(Result_R.size() - 1).begin() + ss, Eigen::Quaterniond(0,0,0,0));
					Result_re_err.at(Result_re_err.size() - 1).insert(Result_re_err.at(Result_re_err.size() - 1).begin() + ss, 0);
					std::vector<cv::Point2f> temp_vec = {};
					Result_Re_map.at(Result_Re_map.size() - 1).insert(Result_Re_map.at(Result_Re_map.size() - 1).begin() + ss, temp_vec);
				}
			}
			Result_update_calib_points.push_back(obj_pt_vec[0][0]);
		}
		else if(obj_pt_vec.size() == 2)
		{
			qApp->processEvents();
			oQProgressDialog.setLabelText(tr("Optimize param for Multi-Camera"));
			oQProgressDialog.setValue(obj_pt_vec.size() + 1);
			qApp->processEvents();
			Eigen::Quaterniond R_now;
			Eigen::Vector3d T_now; 
			std::vector<std::vector<double>> re_err;
			std::vector<std::vector<std::vector<cv::Point2f>>> Re_map;
			std::vector<cv::Point3f> new_point = obj_pt_vec[0][0];
			for (int kk = 1; kk < obj_pt_vec.size(); kk++)
			{
				for (int qq = 0; qq < obj_pt_vec[kk][0].size();qq++)
				{
					new_point[qq].x += obj_pt_vec[kk][0][qq].x;
					new_point[qq].y += obj_pt_vec[kk][0][qq].y;
					new_point[qq].z += obj_pt_vec[kk][0][qq].z;
				}
			}
			for (int qq = 0; qq < new_point.size(); qq++)
			{
				new_point[qq].x /= ((double)(obj_pt_vec.size()));
				new_point[qq].y /= ((double)(obj_pt_vec.size()));
				new_point[qq].z /= ((double)(obj_pt_vec.size()));
			}

			cam_log_ui->ui.textEdit->clear();
			ceres::Solver::Summary summary;
			if (cam_model_ui->ui.optimize_checkBox->isChecked())
			{
				calculate_camera_parame_fixed_optimize_Dual_thread thread_cal(valid_width_size * valid_height_size,
					fix_index, zero_index, img_pt_vec, new_point
					, R_vec, T_vec
					, K_vec, Dis_K_vec, Dis_P_vec, Dis_T_vec, &summary
					, cam_model_ui->ui.max_iter_spinBox->value()
					, ((double)cam_model_ui->ui.stop_spinBox->value())* pow(10, cam_model_ui->ui.stop_spinBox_2->value())
					, cam_model_ui->ui.thread_spinBox->value()
					, cam_model_ui->ui.timeout_spinBox->value()
					, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
					, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.skew_checkBox->isChecked()
					, cam_model_ui->ui.uniform_f_checkBox->isChecked());
				thread_cal.start();
				while (1)
				{
					if (!thread_cal.isRunning())
					{
						break;
					}
					QString inf_log = "";
					for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
					{
						inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
						inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
						inf_log = inf_log + tr("\n");
					}
					cam_log_ui->ui.textEdit->setText(inf_log);
					loop_sleep(20);
				}
				std::vector<double> cost_plot;
				for (int ss = 0; ss < summary.iterations.size(); ss++)
				{
					cost_plot.push_back(summary.iterations[ss].cost);
				}
				K_vec = thread_cal.camK;
				Dis_K_vec = thread_cal.camDK;
				Dis_P_vec = thread_cal.camDP;
				Dis_T_vec = thread_cal.camDT;
				R_now = thread_cal.Dual_R;
				T_now = thread_cal.Dual_T;
				Re_map = thread_cal.Re_project_Map;
				re_err = thread_cal.reproj_err;
				obj_pt_vec[0][0] = thread_cal.obj_pt;
				cam_log_ui->ui.textEdit->clear();

				Eigen::Matrix3d R_now_1;
				R_now_1 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
				Eigen::Vector3d T_now_1;
				T_now_1 << 0, 0, 0;
				for (int ss = 0; ss < 2; ss++)
				{
					Result_K.push_back(K_vec[ss]);
					Result_Dis_K.push_back(Dis_K_vec[ss]);
					Result_Dis_P.push_back(Dis_P_vec[ss]);
					Result_Dis_T.push_back(Dis_T_vec[ss]);
					Result_R.push_back(R_vec[ss]);
					Result_T.push_back(T_vec[ss]);
					Result_Re_map.push_back(Re_map_vec[ss]);
					Result_re_err.push_back(re_err_vec[ss]);
					Result_update_calib_points.push_back(thread_cal.obj_pt);
					for (int tt = 0; tt < Result_isenable.at(Result_isenable.size() - 2).size(); tt++)
					{
						if (!Result_isenable.at(Result_isenable.size() - 2)[tt])
						{
							Result_T.at(Result_T.size() - 1).insert(Result_T.at(Result_T.size() - 1).begin() + tt, Eigen::Vector3d(0, 0, 0));
							Result_R.at(Result_R.size() - 1).insert(Result_R.at(Result_R.size() - 1).begin() + tt, Eigen::Quaterniond(0, 0, 0, 0));
							Result_re_err.at(Result_re_err.size() - 1).insert(Result_re_err.at(Result_re_err.size() - 1).begin() + tt, 0);
							std::vector<cv::Point2f> temp_vec = {};
							Result_Re_map.at(Result_Re_map.size() - 1).insert(Result_Re_map.at(Result_Re_map.size() - 1).begin() + tt, temp_vec);
						}
					}
				}
				Result_costdata_all.at(Result_costdata_all.size() - 2) = cost_plot;
				Result_costdata_all.at(Result_costdata_all.size() - 1) = cost_plot;
				Result_R_overall.push_back(Eigen::Quaterniond(R_now_1));
				Result_R_overall[Result_R_overall.size() - 1].normalize();
				Result_T_overall.push_back(T_now_1);
				Result_R_overall.push_back(Eigen::Quaterniond(R_now));
				Result_R_overall[Result_R_overall.size() - 1].normalize();
				Result_T_overall.push_back(T_now);
			}
			else
			{
				calculate_camera_parame_Dual_thread thread_cal(obj_pt_vec, img_pt_vec
					, R_vec, T_vec
					, K_vec, Dis_K_vec, Dis_P_vec, Dis_T_vec, &summary
					, cam_model_ui->ui.max_iter_spinBox->value()
					, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
					, cam_model_ui->ui.thread_spinBox->value()
					, cam_model_ui->ui.timeout_spinBox->value()
					, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
					, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
					, cam_model_ui->ui.skew_checkBox->isChecked()
					, cam_model_ui->ui.uniform_f_checkBox->isChecked());
				thread_cal.start();
				while (1)
				{
					if (!thread_cal.isRunning())
					{
						break;
					}
					QString inf_log = "";
					for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
					{
						inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
						inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
						inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
						inf_log = inf_log + tr("\n");
					}
					cam_log_ui->ui.textEdit->setText(inf_log);
					loop_sleep(20);
				}
				std::vector<double> cost_plot;
				for (int ss = 0; ss < summary.iterations.size(); ss++)
				{
					cost_plot.push_back(summary.iterations[ss].cost);
				}
				K_vec = thread_cal.camK;
				Dis_K_vec = thread_cal.camDK;
				Dis_P_vec = thread_cal.camDP;
				Dis_T_vec = thread_cal.camDT;
				R_now = thread_cal.Dual_R;
				T_now = thread_cal.Dual_T;
				Re_map = thread_cal.Re_project_Map;
				re_err = thread_cal.reproj_err;
				cam_log_ui->ui.textEdit->clear();

				Eigen::Matrix3d R_now_1;
				R_now_1 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
				Eigen::Vector3d T_now_1;
				T_now_1 << 0, 0, 0;

				for (int ss = 0; ss < 2; ss++)
				{
					Result_K.push_back(K_vec[ss]);
					Result_Dis_K.push_back(Dis_K_vec[ss]);
					Result_Dis_P.push_back(Dis_P_vec[ss]);
					Result_Dis_T.push_back(Dis_T_vec[ss]);
					Result_R.push_back(R_vec[ss]);
					Result_T.push_back(T_vec[ss]);
					Result_Re_map.push_back(Re_map_vec[ss]);
					Result_re_err.push_back(re_err_vec[ss]);
					Result_update_calib_points.push_back(thread_cal.obj_pt[0][0]);
					for (int tt = 0; tt < Result_isenable.at(Result_isenable.size() - 2).size(); tt++)
					{
						if (!Result_isenable.at(Result_isenable.size() - 2)[tt])
						{
							Result_T.at(Result_T.size() - 1).insert(Result_T.at(Result_T.size() - 1).begin() + tt, Eigen::Vector3d(0, 0, 0));
							Result_R.at(Result_R.size() - 1).insert(Result_R.at(Result_R.size() - 1).begin() + tt, Eigen::Quaterniond(0, 0, 0, 0));
							Result_re_err.at(Result_re_err.size() - 1).insert(Result_re_err.at(Result_re_err.size() - 1).begin() + tt, 0);
							std::vector<cv::Point2f> temp_vec = {};
							Result_Re_map.at(Result_Re_map.size() - 1).insert(Result_Re_map.at(Result_Re_map.size() - 1).begin() + tt, temp_vec);
						}
					}
				}
				Result_costdata_all.at(Result_costdata_all.size() - 2) = cost_plot;
				Result_costdata_all.at(Result_costdata_all.size() - 1) = cost_plot;
				Result_R_overall.push_back(Eigen::Quaterniond(R_now_1));
				Result_R_overall[Result_R_overall.size() - 1].normalize();
				Result_T_overall.push_back(T_now_1);
				Result_R_overall.push_back(Eigen::Quaterniond(R_now));
				Result_R_overall[Result_R_overall.size() - 1].normalize();
				Result_T_overall.push_back(T_now);
			}
		}

		//for (int jj = 0; jj < Result_update_calib_points[0].size(); jj++)
		//{
		//	std::cout << Result_update_calib_points[0][jj].x << "\t" << Result_update_calib_points[0][jj].y << "\t" << Result_update_calib_points[0][jj].z << "\n";
		//}

		//valid
		if (cam_model_ui->ui.verify_checkBox->isChecked())
		{
			oQProgressDialog.setWindowTitle(tr("Verify parameter stability..."));
			oQProgressDialog.setLabelText(tr("Establish sampling sequence..."));
			oQProgressDialog.setRange(0, cam_model_ui->ui.desample_number_label->value() + 2);
			oQProgressDialog.setValue(1);
			oQProgressDialog.reset();
			int de_sample_number = ((double)obj_pt_vec[0].size() * (double)cam_model_ui->ui.desample_spinBox->value() / 100.0);
			de_sample_number = de_sample_number < 1 ? 1 : de_sample_number;
			if (obj_pt_vec[0].size() - de_sample_number < 4)
			{
				QMessageBox::warning(this, tr("Verify parameters  stability"), tr("The sample size is less than 4 and cannot be calculated. Please lower the sampling ratio."), QMessageBox::Ok, QMessageBox::Ok);
				Result_K_verify.push_back({});
				Result_Dis_K_verify.push_back({});
				Result_Dis_P_verify.push_back({});
				Result_Dis_T_verify.push_back({});
				Result_R_overall_verify.push_back({});
				Result_T_overall_verify.push_back({});
				Result_update_calib_points_verify.push_back({});
			}
			double len_all = Cal_Combination_num(obj_pt_vec[0].size(), obj_pt_vec[0].size() - de_sample_number);
			std::vector<int> C_all;
			for (int ii = 0; ii < obj_pt_vec[0].size(); ii++)
			{
				C_all.push_back(ii);
			}
			std::vector<std::vector<int>> C_result;
			if (cam_model_ui->ui.desample_number_label->value() >= len_all)
			{
				oQProgressDialog.setLabelText(tr("Establish sampling sequence: The number of samples needed exceeds combination, execute the full combination."));
				oQProgressDialog.setValue(cam_model_ui->ui.desample_number_label->value() + 1);
				oQProgressDialog.reset();
				qApp->processEvents();
				std::vector<int>  C_temp(obj_pt_vec[0].size() - de_sample_number);
				Combination(C_all, C_temp, C_result, 0, obj_pt_vec[0].size() - de_sample_number, obj_pt_vec[0].size() - de_sample_number);
			}
			else
			{
				if ((double)cam_model_ui->ui.desample_number_label->value() / (double)len_all > 0.1)
				{
					oQProgressDialog.setLabelText(tr("Establish sampling sequence: The proportion of sampling numbers is high, secondary sampling after full combination."));
					oQProgressDialog.setValue(cam_model_ui->ui.desample_number_label->value() + 1);
					oQProgressDialog.reset();
					qApp->processEvents();
					std::vector<std::vector<int>> C_result_temp;
					std::vector<int>  C_temp(obj_pt_vec[0].size() - de_sample_number);
					Combination(C_all, C_temp, C_result_temp, 0, obj_pt_vec[0].size() - de_sample_number, obj_pt_vec[0].size() - de_sample_number);

					srand((unsigned int)time(0));
					std::vector<int> sampling_class = knuth_sampling(C_all, cam_model_ui->ui.desample_number_label->value(), len_all);
					for (int pp = 0; pp < sampling_class.size(); pp++)
					{
						C_result.push_back(C_result_temp[sampling_class[pp]]);
					}
				}
				else
				{
					oQProgressDialog.setLabelText(tr("Establish sampling sequence: Random sampling."));
					oQProgressDialog.reset();
					qApp->processEvents();

					srand((unsigned int)time(0));
					while (1)
					{
						oQProgressDialog.setValue(C_result.size() + 1);
						if (C_result.size() == 0)
						{
							C_result.push_back(knuth_sampling(C_all, obj_pt_vec[0].size() - de_sample_number, obj_pt_vec[0].size()));
						}
						else
						{
							std::vector<int> vec_temp = knuth_sampling(C_all, obj_pt_vec[0].size() - de_sample_number, obj_pt_vec[0].size());
							if (!Verify_include(C_result, vec_temp))
							{
								C_result.push_back(vec_temp);
							}
						}
						if (C_result.size() >= cam_model_ui->ui.desample_number_label->value())
						{
							break;
						}
					}
				}
				//
			}



			std::vector<std::vector<double*>> K_vec_verify_all;
			std::vector<std::vector<double*>> Dis_K_vec_verify_all;
			std::vector<std::vector<double*>> Dis_P_vec_verify_all;
			std::vector<std::vector<double*>> Dis_T_vec_verify_all;
			std::vector<std::vector<std::vector<Eigen::Quaterniond>>> R_vec_verify_all;
			std::vector<std::vector<std::vector<Eigen::Vector3d>>> T_vec_verify_all;
			std::vector<std::vector<Eigen::Quaterniond>> R_vec_overall_verify_all;
			std::vector<std::vector<Eigen::Vector3d>> T_vec_overall_verify_all;
			std::vector<std::vector<cv::Point3f>> update_plate_vec_verify_all;

			oQProgressDialog.setLabelText(tr("Verifying..."));
			oQProgressDialog.setRange(0, cam_model_ui->ui.desample_number_label->value() + 2);
			oQProgressDialog.setValue(1);
			oQProgressDialog.reset();
			qApp->processEvents();

			for (int ii = 0; ii < C_result.size(); ii++)
			{
				oQProgressDialog.setValue(ii + 1);

				std::vector<std::vector<std::vector<cv::Point3f>>> obj_pt_vec_verify;
				std::vector<std::vector<std::vector<cv::Point2f>>> img_pt_vec_verify;
				std::vector<std::vector<std::vector<cv::Point2f>>> board_pt_vec_verify;
				for (int kk = 0; kk < obj_pt_vec.size(); kk++)
				{
					std::vector<std::vector<cv::Point3f>> obj_pt_vec_verify_temp;
					std::vector<std::vector<cv::Point2f>> img_pt_vec_verify_temp;
					std::vector<std::vector<cv::Point2f>> board_pt_vec_verify_temp;
					for (int pp = 0; pp < obj_pt_vec[kk].size(); pp++)
					{
						if (Verify_include_index(C_result[ii], pp))
						{
							obj_pt_vec_verify_temp.push_back(obj_pt_vec[kk][pp]);
							img_pt_vec_verify_temp.push_back(img_pt_vec[kk][pp]);
							board_pt_vec_verify_temp.push_back(board_pt_vec[kk][pp]);
						}
					}
					obj_pt_vec_verify.push_back(obj_pt_vec_verify_temp);
					img_pt_vec_verify.push_back(img_pt_vec_verify_temp);
					board_pt_vec_verify.push_back(board_pt_vec_verify_temp);
				}

				std::vector<double*> K_vec_verify;
				std::vector<double*> Dis_K_vec_verify;
				std::vector<double*> Dis_P_vec_verify;
				std::vector<double*> Dis_T_vec_verify;
				std::vector<std::vector<Eigen::Quaterniond>> R_vec_verify;
				std::vector<std::vector<Eigen::Vector3d>> T_vec_verify;
				std::vector<std::vector<cv::Point3f>> update_plate_vec_verify;

				for (int kk = 0; kk < obj_pt_vec.size(); kk++)
				{
					qApp->processEvents();
					if (oQProgressDialog.wasCanceled())
					{
						QMessageBox::information(this, tr("Process"), tr("User Stop"), QMessageBox::Ok);
						return;
					}
					double* K_verify = nullptr;
					double* Dis_K_verify = nullptr;
					double* Dis_P_verify = nullptr;
					double* Dis_T_verify = nullptr;

					cam_log_ui->ui.textEdit->clear();
					ceres::Solver::Summary summary;
					if (cam_model_ui->ui.optimize_checkBox->isChecked())
					{
						calculate_camera_parame_fixed_optimize_thread thread_cal(valid_width_size * valid_height_size,
							valid_width[kk], valid_height[kk],
							fix_index, zero_index,
							img_pt_vec_verify[kk], board_pt_vec_verify[kk], obj_pt_vec_verify[kk][1], &summary
							, cam_model_ui->ui.max_iter_spinBox->value()
							, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
							, cam_model_ui->ui.thread_spinBox->value()
							, cam_model_ui->ui.timeout_spinBox->value()
							, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
							, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.skew_checkBox->isChecked()
							, cam_model_ui->ui.uniform_f_checkBox->isChecked());
						thread_cal.start();

						while (1)
						{
							if (!thread_cal.isRunning())
							{
								break;
							}
							QString inf_log = "";
							for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
							{
								inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
								inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
								inf_log = inf_log + tr("\n");
							}
							cam_log_ui->ui.textEdit->setText(inf_log);
							loop_sleep(20);
						}

						K_vec_verify.push_back(thread_cal.K);
						Dis_K_vec_verify.push_back(thread_cal.Dis_K);
						Dis_P_vec_verify.push_back(thread_cal.Dis_P);
						Dis_T_vec_verify.push_back(thread_cal.Dis_T);
						update_plate_vec_verify.push_back(thread_cal.obj_pt);
						R_vec_verify.push_back(thread_cal.R);
						T_vec_verify.push_back(thread_cal.T);
					}
					else
					{
						calculate_camera_parame_thread thread_cal(valid_width[kk], valid_height[kk],
							img_pt_vec_verify[kk], board_pt_vec_verify[kk], obj_pt_vec_verify[kk], &summary
							, cam_model_ui->ui.max_iter_spinBox->value()
							, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
							, cam_model_ui->ui.thread_spinBox->value()
							, cam_model_ui->ui.timeout_spinBox->value()
							, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
							, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.skew_checkBox->isChecked()
							, cam_model_ui->ui.uniform_f_checkBox->isChecked());
						thread_cal.start();

						while (1)
						{
							if (!thread_cal.isRunning())
							{
								break;
							}
							QString inf_log = "";
							for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
							{
								inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
								inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
								inf_log = inf_log + tr("\n");
							}
							cam_log_ui->ui.textEdit->setText(inf_log);
							loop_sleep(20);
						}

						K_vec_verify.push_back(thread_cal.K);
						Dis_K_vec_verify.push_back(thread_cal.Dis_K);
						Dis_P_vec_verify.push_back(thread_cal.Dis_P);
						Dis_T_vec_verify.push_back(thread_cal.Dis_T);
						update_plate_vec_verify.push_back(thread_cal.obj_pt[0]);
						R_vec_verify.push_back(thread_cal.R);
						T_vec_verify.push_back(thread_cal.T);
					}
				}
				if (obj_pt_vec.size() == 1)
				{
					Eigen::Matrix3d R_now;
					R_now << 1, 0, 0, 0, 1, 0, 0, 0, 1;
					Eigen::Vector3d T_now;
					T_now << 0, 0, 0;
					std::vector<Eigen::Quaterniond> R_overall_verify;
					std::vector<Eigen::Vector3d> T_overall_verify;
					R_overall_verify.push_back(Eigen::Quaterniond(R_now));
					R_overall_verify[R_overall_verify.size() - 1].normalize();
					T_overall_verify.push_back(T_now);

					K_vec_verify_all.push_back(K_vec_verify);
					Dis_K_vec_verify_all.push_back(Dis_K_vec_verify);
					Dis_P_vec_verify_all.push_back(Dis_P_vec_verify);
					Dis_T_vec_verify_all.push_back(Dis_T_vec_verify);
					R_vec_overall_verify_all.push_back(R_overall_verify);
					T_vec_overall_verify_all.push_back(T_overall_verify);
					update_plate_vec_verify_all.push_back(update_plate_vec_verify[0]);
				}
				else if (obj_pt_vec.size() == 2)
				{
					cam_log_ui->ui.textEdit->clear();
					ceres::Solver::Summary summary;
					if (cam_model_ui->ui.optimize_checkBox->isChecked())
					{
						calculate_camera_parame_fixed_optimize_Dual_thread thread_cal(valid_width_size * valid_height_size,
							fix_index, zero_index, img_pt_vec_verify, obj_pt_vec_verify[0][1]
							, R_vec_verify, T_vec_verify
							, K_vec_verify, Dis_K_vec_verify, Dis_P_vec_verify, Dis_T_vec_verify, &summary
							, cam_model_ui->ui.max_iter_spinBox->value()
							, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
							, cam_model_ui->ui.thread_spinBox->value()
							, cam_model_ui->ui.timeout_spinBox->value()
							, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
							, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.skew_checkBox->isChecked()
							, cam_model_ui->ui.uniform_f_checkBox->isChecked());
						thread_cal.start();
						while (1)
						{
							if (!thread_cal.isRunning())
							{
								break;
							}
							QString inf_log = "";
							for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
							{
								inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
								inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
								inf_log = inf_log + tr("\n");
							}
							cam_log_ui->ui.textEdit->setText(inf_log);
							loop_sleep(20);
						}
						cam_log_ui->ui.textEdit->clear();
						Eigen::Matrix3d R_now_1;
						R_now_1 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
						Eigen::Vector3d T_now_1;
						T_now_1 << 0, 0, 0;
						std::vector<Eigen::Quaterniond> R_overall_verify;
						std::vector<Eigen::Vector3d> T_overall_verify;
						R_overall_verify.push_back(Eigen::Quaterniond(R_now_1));
						R_overall_verify[R_overall_verify.size() - 1].normalize();
						T_overall_verify.push_back(T_now_1);
						R_overall_verify.push_back(thread_cal.Dual_R);
						T_overall_verify.push_back(thread_cal.Dual_T);

						K_vec_verify_all.push_back(thread_cal.camK);
						Dis_K_vec_verify_all.push_back(thread_cal.camDK);
						Dis_P_vec_verify_all.push_back(thread_cal.camDP);
						Dis_T_vec_verify_all.push_back(thread_cal.camDT);
						update_plate_vec_verify_all.push_back(thread_cal.obj_pt);
						R_vec_overall_verify_all.push_back(R_overall_verify);
						T_vec_overall_verify_all.push_back(T_overall_verify);
					}
					else
					{
						calculate_camera_parame_Dual_thread thread_cal(obj_pt_vec, img_pt_vec
							, R_vec, T_vec
							, K_vec, Dis_K_vec, Dis_P_vec, Dis_T_vec, &summary
							, cam_model_ui->ui.max_iter_spinBox->value()
							, ((double)cam_model_ui->ui.stop_spinBox->value()) * pow(10, cam_model_ui->ui.stop_spinBox_2->value())
							, cam_model_ui->ui.thread_spinBox->value()
							, cam_model_ui->ui.timeout_spinBox->value()
							, cam_model_ui->ui.comboBox->currentIndex(), cam_model_ui->ui.loss_doubleSpinBox->value()
							, cam_model_ui->ui.k_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.p_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.s_distort_comboBox->currentText().toInt()
							, cam_model_ui->ui.skew_checkBox->isChecked()
							, cam_model_ui->ui.uniform_f_checkBox->isChecked());
						thread_cal.start();
						while (1)
						{
							if (!thread_cal.isRunning())
							{
								break;
							}
							QString inf_log = "";
							for (int ss = summary.iterations.size() - 1; ss >= 0; ss--)
							{
								inf_log = inf_log + QString("%1").arg(ss + 1, 4, 10, QLatin1Char('0')) + tr(". ");
								inf_log = inf_log + tr("[Cost]:") + QString::number(summary.iterations[ss].cost, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Gradient Norm]:") + QString::number(summary.iterations[ss].gradient_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Step Norm]:") + QString::number(summary.iterations[ss].step_norm, 'e', 3) + tr("; ");
								inf_log = inf_log + tr("[Iteration Time(second)]:") + QString::number(summary.iterations[ss].iteration_time_in_seconds, 'e', 3) + tr(";");
								inf_log = inf_log + tr("\n");
							}
							cam_log_ui->ui.textEdit->setText(inf_log);
							loop_sleep(20);
						}
						cam_log_ui->ui.textEdit->clear();
						Eigen::Matrix3d R_now_1;
						R_now_1 << 1, 0, 0, 0, 1, 0, 0, 0, 1;
						Eigen::Vector3d T_now_1;
						T_now_1 << 0, 0, 0;
						std::vector<Eigen::Quaterniond> R_overall_verify;
						std::vector<Eigen::Vector3d> T_overall_verify;
						R_overall_verify.push_back(Eigen::Quaterniond(R_now_1));
						R_overall_verify[R_overall_verify.size() - 1].normalize();
						T_overall_verify.push_back(T_now_1);
						R_overall_verify.push_back(thread_cal.Dual_R);
						T_overall_verify.push_back(thread_cal.Dual_T);

						K_vec_verify_all.push_back(thread_cal.camK);
						Dis_K_vec_verify_all.push_back(thread_cal.camDK);
						Dis_P_vec_verify_all.push_back(thread_cal.camDP);
						Dis_T_vec_verify_all.push_back(thread_cal.camDT);
						update_plate_vec_verify_all.push_back(thread_cal.obj_pt[0][0]);
						R_vec_overall_verify_all.push_back(R_overall_verify);
						T_vec_overall_verify_all.push_back(T_overall_verify);
					}
				}
			}
			for (int ii = 0; ii < K_vec_verify_all[0].size(); ii++)
			{

				std::vector<double*> K_vec_verify_all_temp;
				std::vector<double*> Dis_K_vec_verify_all_temp;
				std::vector<double*> Dis_P_vec_verify_all_temp;
				std::vector<double*> Dis_T_vec_verify_all_temp;
				std::vector<Eigen::Quaterniond> R_vec_overall_verify_all_temp;
				std::vector<Eigen::Vector3d> T_vec_overall_verify_all_temp;
				std::vector<std::vector<cv::Point3f>> Result_update_calib_points_verify_temp;
				for (int jj = 0; jj < K_vec_verify_all.size(); jj++)
				{
					K_vec_verify_all_temp.push_back(K_vec_verify_all[jj][ii]);
					Dis_K_vec_verify_all_temp.push_back(Dis_K_vec_verify_all[jj][ii]);
					Dis_P_vec_verify_all_temp.push_back(Dis_P_vec_verify_all[jj][ii]);
					Dis_T_vec_verify_all_temp.push_back(Dis_T_vec_verify_all[jj][ii]);
					R_vec_overall_verify_all_temp.push_back(R_vec_overall_verify_all[jj][ii]);
					T_vec_overall_verify_all_temp.push_back(T_vec_overall_verify_all[jj][ii]);
					Result_update_calib_points_verify_temp.push_back(update_plate_vec_verify_all[jj]);
				}
				Result_K_verify.push_back(K_vec_verify_all_temp);
				Result_Dis_K_verify.push_back(Dis_K_vec_verify_all_temp);
				Result_Dis_P_verify.push_back(Dis_P_vec_verify_all_temp);
				Result_Dis_T_verify.push_back(Dis_T_vec_verify_all_temp);
				Result_R_overall_verify.push_back(R_vec_overall_verify_all_temp);
				Result_T_overall_verify.push_back(T_vec_overall_verify_all_temp);
				Result_update_calib_points_verify.push_back(Result_update_calib_points_verify_temp);
			}
		}
		else
		{
			Result_K_verify.push_back({});
			Result_Dis_K_verify.push_back({});
			Result_Dis_P_verify.push_back({});
			Result_Dis_T_verify.push_back({});
			Result_R_overall_verify.push_back({});
			Result_T_overall_verify.push_back({});
			Result_update_calib_points_verify.push_back({});
		}
	}
	cam_log_ui->hide(); 
	update_table_view();
}

bool Camera_calibration_module::Verify_include_index(std::vector<int> group, int index)
{
	bool if_exist_same = false;
	for (int ii = 0; ii < group.size(); ii++)
	{
		if (group[ii] == index)
		{
			if_exist_same = true;
			break;
		}
	}
	return if_exist_same;
}

bool Camera_calibration_module::Verify_include(std::vector<std::vector<int>> group, std::vector<int> vec)
{
	bool if_exist_same = false;
	for (int ii = 0; ii < group.size(); ii++)
	{
		if (group[ii].size() != vec.size())
		{
			continue;
		}
		bool same_vec = true;
		for (int jj = 0; jj < group[ii].size(); jj++)
		{
			if (group[ii][jj] != vec[jj])
			{
				same_vec = false;
				break;
			}
		}
		if (same_vec)
		{
			if_exist_same = true;
			break;
		}
	}
	return if_exist_same;
}

std::vector<int> Camera_calibration_module::knuth_sampling(std::vector<int> pNum, int m, int n)
{
	std::vector<int> result;
	for (int i = 0; i < n; i++)
	{
		if (rand() % (n - i) < m)//rand()%(n-i)的取值范围是[0, n-i）
		{
			result.push_back(pNum[i]);
			m--;
		}
	}
	return result;
}
double Camera_calibration_module::Cal_Combination_num(int N, int M)
{
	double ans = 1;
	if (M > N / 2)
	{
		M = N - M;
	}
	for (int i = 1; i <= M; i++) {
		ans = ans * (double)(N - M + i) / (double)i;
	}
	return ans;
}
void Camera_calibration_module::Combination(std::vector<int>& a, std::vector<int>& b, std::vector <std::vector<int>>& result, int l, int m, int M)
{
	int N = a.size();
	if (m == 0) 
	{
		result.push_back(b);
		return;
	}
	for (int i = l; i < N; i++) {
		b[M - m] = a[i];
		Combination(a, b, result, i + 1, m - 1, M);
	}
}

void Camera_calibration_module::clear_all_data()
{
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		Image_serial_name[ii].clear();
		KeyPoint_Enable_serial[ii].clear();
		KeyPoint_serial[ii].clear();
		Result_contours_serial[ii].clear();
		ImageWidth_serial[ii].clear();
		ImageHeight_serial[ii].clear();
		KeyPointWidth_serial[ii].clear();
		KeyPointHeight_serial[ii].clear();
		KeyPointNeedRecal_serial[ii] = true;
	}
	if (Result_K.size() != 0)
	{
		for (int ii = 0; ii < Result_K.size(); ii++)
		{
			delete[]Result_K[ii];
		}
	}
	if (Result_Dis_K.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K.size(); ii++)
		{
			delete[]Result_Dis_K[ii];
		}
	}
	if (Result_Dis_P.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P.size(); ii++)
		{
			delete[]Result_Dis_P[ii];
		}
	}
	if (Result_Dis_T.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T.size(); ii++)
		{
			delete[]Result_Dis_T[ii];
		}
	}
	Result_costdata_single.clear();
	Result_costdata_all.clear();
	Result_K.clear();
	Result_Dis_K.clear();
	Result_Dis_P.clear();
	Result_Dis_T.clear();
	Result_R.clear();
	Result_R_overall.clear();
	Result_T_overall.clear();
	Result_Re_map.clear();
	Result_re_err.clear();
	Result_update_calib_points.clear();
	Result_isenable.clear();
	Result_index.clear();

	if (Result_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_K_verify[ii].size(); jj++)
			{
				delete[] Result_K_verify[ii][jj];
			}
		}
	}
	Result_K_verify.clear();
	if (Result_Dis_K_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_K_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_K_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_K_verify[ii][jj];
			}
		}
	}
	Result_Dis_K_verify.clear();
	if (Result_Dis_P_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_P_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_P_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_P_verify[ii][jj];
			}
		}
	}
	Result_Dis_P_verify.clear();
	if (Result_Dis_T_verify.size() != 0)
	{
		for (int ii = 0; ii < Result_Dis_T_verify.size(); ii++)
		{
			for (int jj = 0; jj < Result_Dis_T_verify[ii].size(); jj++)
			{
				delete[] Result_Dis_T_verify[ii][jj];
			}
		}
	}
	Result_Dis_T_verify.clear();
	Result_R_overall_verify.clear();
	Result_T_overall_verify.clear();
	Result_update_calib_points_verify.clear();

}
void Camera_calibration_module::export_inf_file()
{
	if (ui.save_keypoint_checkBox->isChecked())
	{
		select_path_save_keypoint();
	}
	if (ui.save_calibrate_checkBox->isChecked())
	{
		select_path_save_result();
	}
}
void Camera_calibration_module::view_report_inf()
{
	cam_log_ui->ui.textEdit->clear();
	cam_log_ui->show();
	if (Result_index.size() == 0)
	{
		QString inf_log = tr("No valid calibration results.");
		cam_log_ui->ui.textEdit->setText(inf_log);
		return;
	}
	QString inf_log = tr("--------------------------------------------------------------------------------------\n");
	inf_log = inf_log + tr("Brief Report:\n");
	inf_log = inf_log + tr("--------------------------------------------------------------------------------------\n");
	for (int pp = 0; pp < ui.group_number_spinBox->value(); pp++)
	{
		std::vector<int> camera_index;
		for (int ii = 0; ii < ui.camera_number_spinBox->value(); ii++)
		{
			if ((Camera_combobox[ii]->currentIndex() + 1) < 1 || (Camera_combobox[ii]->currentIndex() + 1) >= MAX_CALIBRAT_CAMERA)
			{
				continue;
			}
			if ((Camera_combobox[ii]->currentIndex()) == pp)
			{
				camera_index.push_back(ii);
			}
		}
		std::vector<int> camera_valid_index;
		std::vector<int> camera_valid_number;
		for (int ii = 0; ii < camera_index.size(); ii++)
		{
			if (Image_serial_name[camera_index[ii]].size() != 0)
			{
				camera_valid_index.push_back(camera_index[ii]);
				camera_valid_number.push_back(Image_serial_name[camera_index[ii]].size());
			}
		}
		if (camera_valid_number.size() != 0)
		{
			inf_log = inf_log + tr("---------------------------------------------------------------------------------------------------------------------------------\n");
			inf_log = inf_log + tr("Calibration Group:") + QString::number(pp + 1) + "\n"; 
			for (int ii = 0; ii < camera_valid_index.size(); ii++)
			{
				for (int jj = 0; jj < Result_index.size(); jj++)
				{
					if (camera_valid_index[ii] == Result_index[jj])
					{
						inf_log = inf_log + tr("-------------------------------------------\n");
						inf_log = inf_log + tr("-Camera:") + QString::number(camera_valid_index[ii] + 1) + "\n";
						inf_log = inf_log + tr("--Intrinsics([fx,s,cx;0,fy,cy;0,0,1]): [") + 
							QString::number(Result_K[jj][0], 'g', 10) + ", " +
							QString::number(Result_K[jj][4], 'g', 10) + ", " +
							QString::number(Result_K[jj][2], 'g', 10) + "; " +
							QString::number(0) + ", " +
							QString::number(Result_K[jj][1], 'g', 10) + ", " +
							QString::number(Result_K[jj][3], 'g', 10) + "; " +
							QString::number(0) + ", " +
							QString::number(0) + ", " +
							QString::number(1) + "]" + "\n";
						inf_log = inf_log + tr("--Radial distortion([K1,K2,K3,K4,K5,K6]): [") +
							QString::number(Result_Dis_K[jj][0], 'g', 10) + ", " +
							QString::number(Result_Dis_K[jj][1], 'g', 10) + ", " +
							QString::number(Result_Dis_K[jj][2], 'g', 10) + ", " +
							QString::number(Result_Dis_K[jj][3], 'g', 10) + ", " +
							QString::number(Result_Dis_K[jj][4], 'g', 10) + ", " +
							QString::number(Result_Dis_K[jj][5], 'g', 10) + "]" + "\n";
						inf_log = inf_log + tr("--Tangential distortion([P1,P2]): [") +
							QString::number(Result_Dis_P[jj][0], 'g', 10) + ", " +
							QString::number(Result_Dis_P[jj][1], 'g', 10) + "]" + "\n";
						inf_log = inf_log + tr("--Thin prism distortion([S1,S2,S3,S4]): [") +
							QString::number(Result_Dis_T[jj][0], 'g', 10) + ", " +
							QString::number(Result_Dis_T[jj][1], 'g', 10) + ", " +
							QString::number(Result_Dis_T[jj][2], 'g', 10) + ", " +
							QString::number(Result_Dis_T[jj][3], 'g', 10) + "]" + "\n";
						Eigen::Matrix3d rotation_matrix;
						rotation_matrix = Result_R_overall[jj].matrix();
						inf_log = inf_log + tr("--Rotation matrix([3-3]): [") +
							QString::number(rotation_matrix(0, 0), 'g', 10) + ", " +
							QString::number(rotation_matrix(0, 1), 'g', 10) + ", " +
							QString::number(rotation_matrix(0, 2), 'g', 10) + "; " +
							QString::number(rotation_matrix(1, 0), 'g', 10) + ", " +
							QString::number(rotation_matrix(1, 1), 'g', 10) + ", " +
							QString::number(rotation_matrix(1, 2), 'g', 10) + "; " +
							QString::number(rotation_matrix(2, 0), 'g', 10) + ", " +
							QString::number(rotation_matrix(2, 1), 'g', 10) + ", " +
							QString::number(rotation_matrix(2, 2), 'g', 10)  + "]" + "\n";
						inf_log = inf_log + tr("--Translation matrix([3-1]): [") +
							QString::number(Result_T_overall[jj](0), 'g', 10) + "; " +
							QString::number(Result_T_overall[jj](1), 'g', 10) + "; " +
							QString::number(Result_T_overall[jj](2), 'g', 10) + "]" + "\n";
					}
				}
			}
		}
	}

	cam_log_ui->ui.textEdit->setText(inf_log);
}
void Camera_calibration_module::select_path_save_keypoint()
{
	switch (ui.save_type_comboBox->currentIndex())
	{
	case 0:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save KeyPoints data"), tr(""), "MSDATA(*.msdata)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_KeyPoint_file(file_for_msdata);
	}
		break;
	case 1:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save KeyPoints data"), tr(""), "CSV(*.csv)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_KeyPoint_file_csv(file_for_msdata);
	}
		break;
	case 2:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save KeyPoints data"), tr(""), "TXT(*.txt)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_KeyPoint_file_txt(file_for_msdata);
	}
		break;
	case 3:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save KeyPoints data"), tr(""), "MAT(*.mat)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_KeyPoint_file_mat(file_for_msdata);
	}
		break;
	default:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save KeyPoints data"), tr(""), "MSDATA(*.msdata)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_KeyPoint_file(file_for_msdata);
	}
		break;
	}
}
void Camera_calibration_module::select_path_read_keypoint()
{
	QString file_for_msdata = QFileDialog::getOpenFileName(this, tr("Select KeyPoints data"), tr(""), "MSDATA(*.msdata)");
	if (file_for_msdata.isEmpty())
	{
		return;
	}
	clear_all_data();
	read_KeyPoint_file(file_for_msdata);
}
void Camera_calibration_module::select_path_save_result()
{

	switch (ui.save_type_comboBox->currentIndex())
	{
	case 0:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save Result data"), tr(""), "MSDATA(*.msdata)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_CalibrationeResult_file(file_for_msdata);
	}
	break;
	case 1:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save Result data"), tr(""), "CSV(*.csv)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_CalibrationeResult_file_csv(file_for_msdata);
	}
	break;
	case 2:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save Result data"), tr(""), "TXT(*.txt)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_CalibrationeResult_file_txt(file_for_msdata);
	}
	break;
	case 3:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save Result data"), tr(""), "MAT(*.mat)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_CalibrationeResult_file_mat(file_for_msdata);
	}
	break;
	default:
	{
		QString file_for_msdata = QFileDialog::getSaveFileName(this, tr("Save Result data"), tr(""), "MSDATA(*.msdata)");
		if (file_for_msdata.isEmpty())
		{
			return;
		}
		save_CalibrationeResult_file(file_for_msdata);
	}
	break;
	}
}
void Camera_calibration_module::select_path_read_result()
{
	QString file_for_msdata = QFileDialog::getOpenFileName(this, tr("Select KeyPoints data"), tr(""), "MSDATA(*.msdata)");
	if (file_for_msdata.isEmpty())
	{
		return;
	}
	clear_all_data();
	read_CalibrationeResult_file(file_for_msdata);
}

void Camera_calibration_module::read_CalibrationeResult_file(QString path_read)
{
	in_reading_keypoint = true;
	FILE* fp;
	size_t pointer_in_file = 0;
	fp = fopen(path_read.toLocal8Bit().data(), "rb");
	if (NULL == fp)
	{
		QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	unsigned char read_file_mode = -1;
	size_t return_size = fread(&read_file_mode, sizeof(unsigned char), 1, fp);
	if (return_size != 1)
	{
		QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	if (read_file_mode != 2 && read_file_mode != 4 && read_file_mode != 6 && read_file_mode != 8)
	{
		QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Storage content not Calibration Result data!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	if (read_file_mode == 2)
	{
		cur_target_type_cal = Target_type::Chess_Board_type;
	}
	else if (read_file_mode == 4)
	{
		cur_target_type_cal = Target_type::Circle_Board_type;
	}
	else if (read_file_mode == 6)
	{
		cur_target_type_cal = Target_type::Ori_Circle_Board_type;
	}
	else if (read_file_mode == 8)
	{
		cur_target_type_cal = Target_type::Speckle_Board_type;
	}
	if (feof(fp))
	{
		QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Successfully, but no data"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	while (!feof(fp))
	{
		int group_index;
		int camera_index;
		return_size = fread(&group_index, 1, sizeof(int), fp);
		if (return_size != sizeof(int))
		{
			if (feof(fp))
			{
				break;
			}
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			//clear_all_data();
			fclose(fp);
			break;
		}
		if ((group_index + 1) > ui.group_number_spinBox->value())
		{
			ui.group_number_spinBox->setValue(group_index + 1);
		}
		return_size = fread(&camera_index, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		if ((camera_index + 1) > ui.camera_number_spinBox->value())
		{
			ui.camera_number_spinBox->setValue(camera_index + 1);
		}
		Camera_combobox[camera_index]->setCurrentIndex(group_index);
		Result_index.push_back(camera_index);
		double* camera_K = new double[5];
		return_size = fread(camera_K, sizeof(double), 5, fp);
		if (return_size != 5)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Result_K.push_back(camera_K);

		double* camera_Dis_K = new double[6];
		return_size = fread(camera_Dis_K, sizeof(double), 6, fp);
		if (return_size != 6)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Result_Dis_K.push_back(camera_Dis_K);

		double* camera_Dis_P = new double[2];
		return_size = fread(camera_Dis_P, sizeof(double), 2, fp);
		if (return_size != 2)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Result_Dis_P.push_back(camera_Dis_P);

		double* camera_Dis_T = new double[4];
		return_size = fread(camera_Dis_T, sizeof(double), 4, fp);
		if (return_size != 4)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Result_Dis_T.push_back(camera_Dis_T);

		double* camera_R_overall_temp = new double[4];
		return_size = fread(camera_R_overall_temp, sizeof(double), 4, fp);
		if (return_size != 4)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Eigen::Quaterniond camera_R_overall(camera_R_overall_temp[0], camera_R_overall_temp[1], camera_R_overall_temp[2], camera_R_overall_temp[3]);
		delete[] camera_R_overall_temp;
		Result_R_overall.push_back(camera_R_overall);

		double* camera_T_overall_temp = new double[3];
		return_size = fread(camera_T_overall_temp, sizeof(double), 3, fp);
		if (return_size != 3)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Eigen::Vector3d camera_T_overall(camera_T_overall_temp[0], camera_T_overall_temp[1], camera_T_overall_temp[2]);
		delete[] camera_T_overall_temp;
		Result_T_overall.push_back(camera_T_overall);


		int verify_size = -1;
		return_size = fread(&verify_size, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}


		std::vector<double*> Result_K_verify_only;
		std::vector<double* > Result_Dis_K_verify_only;
		std::vector<double* > Result_Dis_P_verify_only;
		std::vector<double* > Result_Dis_T_verify_only;
		std::vector<Eigen::Quaterniond> Result_R_overall_verify_only;
		std::vector<Eigen::Vector3d> Result_T_overall_verify_only;
		std::vector<std::vector<cv::Point3f>> Result_update_calib_points_verify_only;

		for (int jj = 0; jj < verify_size; jj++)
		{
			double* camera_K_verify = new double[5];
			return_size = fread(camera_K_verify, sizeof(double), 5, fp);
			if (return_size != 5)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Result_K_verify_only.push_back(camera_K_verify);

			double* camera_Dis_K_verify = new double[6];
			return_size = fread(camera_Dis_K_verify, sizeof(double), 6, fp);
			if (return_size != 6)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Result_Dis_K_verify_only.push_back(camera_Dis_K_verify);

			double* camera_Dis_P_verify = new double[2];
			return_size = fread(camera_Dis_P_verify, sizeof(double), 2, fp);
			if (return_size != 2)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Result_Dis_P_verify_only.push_back(camera_Dis_P_verify);

			double* camera_Dis_T_verify = new double[4];
			return_size = fread(camera_Dis_T_verify, sizeof(double), 4, fp);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Result_Dis_T_verify_only.push_back(camera_Dis_T_verify);

			double* camera_R_overall_temp_verify = new double[4];
			return_size = fread(camera_R_overall_temp_verify, sizeof(double), 4, fp);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Eigen::Quaterniond camera_R_overall_verify(camera_R_overall_temp_verify[0], camera_R_overall_temp_verify[1],
				camera_R_overall_temp_verify[2], camera_R_overall_temp_verify[3]);
			delete[] camera_R_overall_temp_verify;
			Result_R_overall_verify_only.push_back(camera_R_overall_verify);

			double* camera_T_overall_temp_verify = new double[3];
			return_size = fread(camera_T_overall_temp_verify, sizeof(double), 3, fp);
			if (return_size != 3)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Eigen::Vector3d camera_T_overall_verify(camera_T_overall_temp_verify[0], camera_T_overall_temp_verify[1], camera_T_overall_temp_verify[2]);
			delete[] camera_T_overall_temp_verify;
			Result_T_overall_verify_only.push_back(camera_T_overall_verify);

			int calibrator_corn_number_verify = -1;
			return_size = fread(&calibrator_corn_number_verify, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			std::vector<cv::Point3f> camera_update_calib_points_verify;
			for (int jj = 0; jj < calibrator_corn_number_verify; jj++)
			{
				double temp_x, temp_y, temp_z;
				return_size = fread(&temp_x, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&temp_y, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&temp_z, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				camera_update_calib_points_verify.push_back(cv::Point3f(temp_x, temp_y, temp_z));
			}
			Result_update_calib_points_verify_only.push_back(camera_update_calib_points_verify);

		}
		Result_K_verify.push_back(Result_K_verify_only);
		Result_Dis_K_verify.push_back(Result_Dis_K_verify_only);
		Result_Dis_P_verify.push_back(Result_Dis_P_verify_only);
		Result_Dis_T_verify.push_back(Result_Dis_T_verify_only);
		Result_R_overall_verify.push_back(Result_R_overall_verify_only);
		Result_T_overall_verify.push_back(Result_T_overall_verify_only);

		double reprojector_error_for_camera = -1;
		return_size = fread(&reprojector_error_for_camera, sizeof(double), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}

		int itera_number_single = -1;
		return_size = fread(&itera_number_single, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		std::vector<double> cost_temp_single;
		for (int jj = 0; jj < itera_number_single; jj++)
		{
			double cost_val_temp;
			return_size = fread(&cost_val_temp, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			cost_temp_single.push_back(cost_val_temp);
		}
		Result_costdata_single.push_back(cost_temp_single);


		int itera_number_all = -1;
		return_size = fread(&itera_number_all, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		std::vector<double> cost_temp_all;
		for (int jj = 0; jj < itera_number_all; jj++)
		{
			double cost_val_temp;
			return_size = fread(&cost_val_temp, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			cost_temp_all.push_back(cost_val_temp);
		}
		Result_costdata_all.push_back(cost_temp_all);

		int calibrator_corn_number = -1;
		return_size = fread(&calibrator_corn_number, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		std::vector<cv::Point3f> camera_update_calib_points;
		for (int jj = 0; jj < calibrator_corn_number; jj++)
		{
			double temp_x, temp_y, temp_z;
			return_size = fread(&temp_x, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&temp_y, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&temp_z, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			camera_update_calib_points.push_back(cv::Point3f(temp_x, temp_y, temp_z));
		}
		Result_update_calib_points.push_back(camera_update_calib_points);
		int number_for_camera = -1;
		return_size = fread(&number_for_camera, sizeof(int), 1, fp);

		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}

		std::vector<Eigen::Quaterniond> camera_R;
		std::vector<Eigen::Vector3d> camera_T;
		std::vector<std::vector<cv::Point2f>> camera_Re_map;
		std::vector<double> camera_re_err;
		std::vector<bool> camera_isenable;

		for (int ii = 0; ii < number_for_camera; ii++)
		{
			int image_index;
			return_size = fread(&image_index, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			bool camera_enable_only;
			return_size = fread(&camera_enable_only, sizeof(bool), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			camera_isenable.push_back(camera_enable_only);

			double reproj_error_only;
			return_size = fread(&reproj_error_only, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			if (!camera_enable_only || reproj_error_only == -1)
			{
				reproj_error_only = 0;
			}
			camera_re_err.push_back(reproj_error_only);
			double* camera_R_overall_temp_only = new double[4];
			return_size = fread(camera_R_overall_temp_only, sizeof(double), 4, fp);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Eigen::Quaterniond camera_R_overall_only(camera_R_overall_temp_only[0], camera_R_overall_temp_only[1], camera_R_overall_temp_only[2], camera_R_overall_temp_only[3]);
			delete[] camera_R_overall_temp_only;
			camera_R.push_back(camera_R_overall_only);

			double* camera_T_overall_temp_only = new double[3];
			return_size = fread(camera_T_overall_temp_only, sizeof(double), 3, fp);
			if (return_size != 3)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Eigen::Vector3d camera_T_overall_only(camera_T_overall_temp_only[0], camera_T_overall_temp_only[1], camera_T_overall_temp_only[2]);
			delete[] camera_T_overall_temp_only;
			camera_T.push_back(camera_T_overall_only);

			long long string_path_length;
			return_size = fread(&string_path_length, sizeof(long long), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			std::string string_path;
			string_path.resize(string_path_length);
			return_size = fread(&string_path[0], sizeof(char), string_path_length, fp);
			if (return_size != string_path_length)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			Image_serial_name[camera_index].push_back(QString(QString::fromLocal8Bit(string_path.data())));
			KeyPoint_Enable_serial[camera_index].push_back(true);
			QDir file_temp = QDir(QString(QString::fromLocal8Bit(string_path.data())));
			file_temp.cdUp();
			Camera_lineedit[camera_index]->setText(file_temp.absolutePath());
			int image_width = -1, image_height = -1;
			return_size = fread(&image_width, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&image_height, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			ImageWidth_serial[camera_index].push_back(image_width);
			ImageHeight_serial[camera_index].push_back(image_height);
			int corn_size = -1, corn_w_size = -1, corn_h_size = -1;
			return_size = fread(&corn_size, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&corn_w_size, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&corn_h_size, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			if (corn_size != (corn_w_size * corn_h_size))
			{
				QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read failed to analysis for KeyPoints numbers!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			KeyPointWidth_serial[camera_index].push_back(corn_w_size);
			KeyPointHeight_serial[camera_index].push_back(corn_h_size);
			if (cur_target_type_cal == Target_type::Ori_Circle_Board_type)
			{
				return_size = fread(&cur_target_offset_for_ori_wid, sizeof(int), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&cur_target_offset_for_ori_hei, sizeof(int), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&cur_target_incre_for_ori_wid, sizeof(int), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&cur_target_incre_for_ori_hei, sizeof(int), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
			}
			std::vector<cv::Point2f> kp_temp;
			std::vector<cv::Point2f> kp_err_temp;
			for (int pp = 0; pp < corn_size; pp++)
			{
				double temp_x, temp_y, temp_err_x, temp_err_y;
				return_size = fread(&temp_x, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&temp_y, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&temp_err_x, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					fclose(fp);
					return;
				}
				return_size = fread(&temp_err_y, sizeof(double), 1, fp);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Read Calibration Result File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					clear_all_data();
					update_show_view();
					in_reading_keypoint = false;
					update_table_view();
					fclose(fp);
					for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
					{
						Camera_lineedit[ii]->setText(tr(""));
					}
					return;
				}
				if (temp_err_x == std::numeric_limits<double>::max() || temp_err_y == std::numeric_limits<double>::max() || !camera_enable_only)
				{
					temp_err_x = std::numeric_limits<double>::max();
					temp_err_y = std::numeric_limits<double>::max();
				}
				kp_temp.push_back(cv::Point2f(temp_x, temp_y));
				kp_err_temp.push_back(cv::Point2f(temp_err_x, temp_err_y));
			}
			KeyPoint_serial[camera_index].push_back(kp_temp);
			camera_Re_map.push_back(kp_err_temp);
		}
		Result_R.push_back(camera_R);
		Result_T.push_back(camera_T);
		Result_Re_map.push_back(camera_Re_map);
		Result_re_err.push_back(camera_re_err);
		Result_isenable.push_back(camera_isenable);
	}
	
	if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 0)
	{
		ui.show_detect_spinBox->setEnabled(false);
		ui.show_detect_horizontalSlider->setEnabled(false);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(1);
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(1);
		ui.show_detect_spinBox->setValue(1);
		ui.show_detect_horizontalSlider->setValue(1);
		ui.Detect_view->clear_image();
	}
	else
	{
		ui.show_detect_spinBox->setEnabled(true);
		ui.show_detect_horizontalSlider->setEnabled(true);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
		ui.show_detect_spinBox->setValue(1);
		ui.show_detect_horizontalSlider->setValue(1);
	}

	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		KeyPointNeedRecal_serial[ii] = false;
	}
	in_reading_keypoint = false;
	update_show_view();
	update_table_view();
	QMessageBox::information(this, tr("Read KeyPoint File"), tr("Read Successfully."), QMessageBox::Ok);
	fclose(fp);
}
void Camera_calibration_module::read_KeyPoint_file(QString path_read)
{
	in_reading_keypoint = true;
	FILE* fp;
	size_t pointer_in_file = 0;
	fp = fopen(path_read.toLocal8Bit().data(), "rb");
	if (NULL == fp)
	{
		QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		in_reading_keypoint = false;
		update_show_view();
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	unsigned char read_file_mode = -1;
	size_t return_size = fread(&read_file_mode, sizeof(unsigned char), 1, fp);
	if (return_size != 1)
	{
		QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	if (read_file_mode != 1 && read_file_mode != 3 && read_file_mode != 5 && read_file_mode != 7)
	{
		QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Storage content not KeyPoint data!"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	if (read_file_mode == 1)
	{
		cur_target_type_cal = Target_type::Chess_Board_type;
	}
	else if (read_file_mode == 3)
	{
		cur_target_type_cal = Target_type::Circle_Board_type;
	}
	else if (read_file_mode == 5)
	{
		cur_target_type_cal = Target_type::Ori_Circle_Board_type;
	}
	else if (read_file_mode == 7)
	{
		cur_target_type_cal = Target_type::Speckle_Board_type;
	}
	if (feof(fp))
	{
		QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Successfully, but no data"), QMessageBox::Ok, QMessageBox::Ok);
		clear_all_data();
		update_show_view();
		in_reading_keypoint = false;
		update_table_view();
		for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
		{
			Camera_lineedit[ii]->setText(tr(""));
		}
		fclose(fp);
		return;
	}
	while (!feof(fp))
	{
		int group_index;
		int camera_index;
		int image_index;
		return_size = fread(&group_index, 1, sizeof(int), fp);
		if (return_size != sizeof(int))
		{
			if (feof(fp))
			{
				break;
			}
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			//clear_all_data();
			fclose(fp);
			break;
		}
		if ((group_index + 1) > ui.group_number_spinBox->value())
		{
			ui.group_number_spinBox->setValue(group_index + 1);
		}
		return_size = fread(&camera_index, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		if ((camera_index + 1) > ui.camera_number_spinBox->value())
		{
			ui.camera_number_spinBox->setValue(camera_index + 1);
		}
		Camera_combobox[camera_index]->setCurrentIndex(group_index);
		return_size = fread(&image_index, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		long long string_path_length;
		return_size = fread(&string_path_length, sizeof(long long), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		std::string string_path;
		string_path.resize(string_path_length);
		return_size = fread(&string_path[0], sizeof(char), string_path_length, fp);
		if (return_size != string_path_length)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		Image_serial_name[camera_index].push_back(QString(QString::fromLocal8Bit(string_path.data())));
		KeyPoint_Enable_serial[camera_index].push_back(true);
		QDir file_temp = QDir(QString(QString::fromLocal8Bit(string_path.data())));
		file_temp.cdUp();
		Camera_lineedit[camera_index]->setText(file_temp.absolutePath());
		int image_width = -1, image_height = -1;
		return_size = fread(&image_width, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		return_size = fread(&image_height, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		ImageWidth_serial[camera_index].push_back(image_width);
		ImageHeight_serial[camera_index].push_back(image_height);
		int corn_size = -1, corn_w_size = -1, corn_h_size = -1;
		return_size = fread(&corn_size, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		return_size = fread(&corn_w_size, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		return_size = fread(&corn_h_size, sizeof(int), 1, fp);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		if (corn_size != (corn_w_size * corn_h_size))
		{
			QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read failed to analysis for KeyPoints numbers!"), QMessageBox::Ok, QMessageBox::Ok);
			clear_all_data();
			update_show_view();
			in_reading_keypoint = false;
			update_table_view();
			for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
			{
				Camera_lineedit[ii]->setText(tr(""));
			}
			fclose(fp);
			return;
		}
		KeyPointWidth_serial[camera_index].push_back(corn_w_size);
		KeyPointHeight_serial[camera_index].push_back(corn_h_size);
		if (cur_target_type_cal == Target_type::Ori_Circle_Board_type)
		{
			return_size = fread(&cur_target_offset_for_ori_wid, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&cur_target_offset_for_ori_hei, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&cur_target_incre_for_ori_wid, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&cur_target_incre_for_ori_hei, sizeof(int), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
		}
		std::vector<cv::Point2f> kp_temp;
		for (int pp = 0; pp < corn_size; pp++)
		{
			double temp_x, temp_y;
			return_size = fread(&temp_x, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			return_size = fread(&temp_y, sizeof(double), 1, fp);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Read KeyPoint File"), tr("Read Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				clear_all_data();
				update_show_view();
				in_reading_keypoint = false;
				update_table_view();
				for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
				{
					Camera_lineedit[ii]->setText(tr(""));
				}
				fclose(fp);
				return;
			}
			kp_temp.push_back(cv::Point2f(temp_x, temp_y));
		}
		KeyPoint_serial[camera_index].push_back(kp_temp);
	}

	if (Image_serial_name[ui.curren_index_spinBox->value() - 1].size() == 0)
	{
		ui.show_detect_spinBox->setEnabled(false);
		ui.show_detect_horizontalSlider->setEnabled(false);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(1);
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(1);
		ui.show_detect_spinBox->setValue(1);
		ui.show_detect_horizontalSlider->setValue(1);
		ui.Detect_view->clear_image();
	}
	else
	{
		ui.show_detect_spinBox->setEnabled(true);
		ui.show_detect_horizontalSlider->setEnabled(true);
		ui.show_detect_spinBox->setMinimum(1);
		ui.show_detect_spinBox->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
		ui.show_detect_horizontalSlider->setMinimum(1);
		ui.show_detect_horizontalSlider->setMaximum(Image_serial_name[ui.curren_index_spinBox->value() - 1].size());
		ui.show_detect_spinBox->setValue(1);
		ui.show_detect_horizontalSlider->setValue(1);
	}

	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		KeyPointNeedRecal_serial[ii] = false;
	}
	in_reading_keypoint = false;
	update_show_view();
	update_table_view();
	QMessageBox::information(this, tr("Read KeyPoint File"), tr("Read Successfully."), QMessageBox::Ok);
	fclose(fp);
}

void Camera_calibration_module::save_KeyPoint_file(QString path_save)
{
	FILE* f = fopen(path_save.toLocal8Bit().data(), "wb");
	unsigned char save_mode = 1;
	switch (cur_target_type_cal)
	{
	case Chess_Board_type:
		save_mode = 1;
		break;
	case Circle_Board_type:
		save_mode = 3;
		break;
	case Ori_Circle_Board_type:
		save_mode = 5;
		break;
	case Speckle_Board_type:
		save_mode = 7;
		break;
	default:
		save_mode = 1;
		break;
	}
	size_t return_size = fwrite(&save_mode, sizeof(unsigned char), 1, f);
	if (return_size != 1)
	{
		QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		fclose(f);
		return;
	}
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		int camera_index = ii;
		int group_index = Camera_combobox[ii]->currentIndex();
		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			int image_index = jj;
			return_size = fwrite(&group_index, sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&camera_index, sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&image_index, sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			long long string_len = Image_serial_name[ii][jj].toLocal8Bit().toStdString().size();
			return_size = fwrite(&string_len, sizeof(long long), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			std::string im_path = Image_serial_name[ii][jj].toLocal8Bit().toStdString();
			return_size = fwrite(&im_path[0], sizeof(char), string_len, f);
			if (return_size != string_len)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&ImageWidth_serial[ii][jj], sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&ImageHeight_serial[ii][jj], sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			int corn_number = KeyPoint_serial[ii][jj].size();
			if (corn_number == KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj])
			{
				return_size = fwrite(&corn_number, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&KeyPointWidth_serial[ii][jj], sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&KeyPointHeight_serial[ii][jj], sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				if (cur_target_type_cal == Target_type::Ori_Circle_Board_type)
				{
					return_size = fwrite(&cur_target_offset_for_ori_wid, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_offset_for_ori_hei, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_incre_for_ori_wid, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_incre_for_ori_hei, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
				}
				for (int pp = 0; pp < corn_number; pp++)
				{
					double corn_x = KeyPoint_serial[ii][jj][pp].x;
					double corn_y = KeyPoint_serial[ii][jj][pp].y;
					return_size = fwrite(&corn_x, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&corn_y, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
				}
			}
			else
			{
				corn_number = 0;
				int corn_w = 0;
				int corn_h = 0;
				return_size = fwrite(&corn_number, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&corn_w, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&corn_h, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
			}
		}
	}
	QMessageBox::information(this, tr("Save KeyPoint File"), tr("Save Successfully."), QMessageBox::Ok);
	fclose(f);
}
void Camera_calibration_module::save_KeyPoint_file_csv(QString path_save)
{
	QFile file(path_save);
	if (!file.exists())  //文件不存在的时新建
	{
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream txtOutPut(&file);
		file.close();
	}
	file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	QTextStream txtOutPut(&file);
	txtOutPut.setEncoding(QStringConverter::System);
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		if (Image_serial_name[ii].size() == 0)
		{
			continue;
		}
		txtOutPut << tr("Camera-Index-Group") << "," << tr("Camera-") << QString::number(ii + 1) << "," << tr("Group-") << QString::number(Camera_combobox[ii]->currentIndex() + 1) << "\n";
		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			txtOutPut << Image_serial_name[ii][jj].toLocal8Bit() << ",";
			txtOutPut << ImageWidth_serial[ii][jj] << ",";
			txtOutPut << ImageHeight_serial[ii][jj] << ",";
			int corn_number = KeyPoint_serial[ii][jj].size();
			if (corn_number == KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj])
			{
				txtOutPut << corn_number << ",";
				txtOutPut << KeyPointWidth_serial[ii][jj] << ",";
				txtOutPut << KeyPointHeight_serial[ii][jj] << "\n";
				txtOutPut << "" << "," << "X" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].x << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].x << ",";
					}
				}
				txtOutPut << "" << "," << "Y" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].y << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].y << ",";
					}
				}
			}
			else
			{
				txtOutPut << 0 << ",";
				txtOutPut << 0 << ",";
				txtOutPut << 0 << "\n";
				txtOutPut << "\n";
			}
		}
	}
	QMessageBox::information(this, tr("Save KeyPoint File"), tr("Save Successfully."), QMessageBox::Ok);
	file.close();
}
void Camera_calibration_module::save_KeyPoint_file_txt(QString path_save)
{
	QFile file(path_save);
	if (!file.exists())  //文件不存在的时新建
	{
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream txtOutPut(&file);
		file.close();
	}
	file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	QTextStream txtOutPut(&file);
	txtOutPut.setEncoding(QStringConverter::System);
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		if (Image_serial_name[ii].size() == 0)
		{
			continue;
		}
		txtOutPut << tr("Camera-Index-Group") << "\t" << tr("Camera-") << QString::number(ii + 1) << "\t" << tr("Group-") << QString::number(Camera_combobox[ii]->currentIndex() + 1) << "\n";
		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			txtOutPut << Image_serial_name[ii][jj].toLocal8Bit() << "\t";
			txtOutPut << ImageWidth_serial[ii][jj] << "\t";
			txtOutPut << ImageHeight_serial[ii][jj] << "\t";
			int corn_number = KeyPoint_serial[ii][jj].size();
			if (corn_number == KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj])
			{
				txtOutPut << corn_number << "\t";
				txtOutPut << KeyPointWidth_serial[ii][jj] << "\t";
				txtOutPut << KeyPointHeight_serial[ii][jj] << "\n";
				txtOutPut << "" << "\t" << "X" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].x << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].x << "\t";
					}
				}
				txtOutPut << "" << "\t" << "Y" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].y << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[ii][jj][pp].y << "\t";
					}
				}
			}
			else
			{
				txtOutPut << 0 << "\t";
				txtOutPut << 0 << "\t";
				txtOutPut << 0 << "\n";
				txtOutPut << "\n";
			}
		}
	}
	QMessageBox::information(this, tr("Save KeyPoint File"), tr("Save Successfully."), QMessageBox::Ok);
	file.close();
}
void Camera_calibration_module::save_KeyPoint_file_mat(QString path_save)
{
	MATFile* pmatFile = NULL;
	pmatFile = matOpen(path_save.toLocal8Bit().data(), "w");
	if (pmatFile == NULL)
	{
		QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}

	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		if (Image_serial_name[ii].size() == 0)
		{
			continue;
		}
		int max_length = 0;
		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			if (KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj] == KeyPoint_serial[ii][jj].size())
			{
				if (max_length < KeyPoint_serial[ii][jj].size())
				{
					max_length = KeyPoint_serial[ii][jj].size();
				}
			}
		}
		max_length++;
		max_length = max_length < 1 ? 1 : max_length;
		mxArray* pMxArray = NULL;
		pMxArray = mxCreateDoubleMatrix(max_length, Image_serial_name[ii].size() * 2, mxREAL);
		if (pMxArray == NULL)
		{
			QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data = new double[Image_serial_name[ii].size() * 2 * max_length];
		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			if (KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj] == KeyPoint_serial[ii][jj].size() && max_length!=1)
			{
				out_data[max_length * 2 * jj] = KeyPointWidth_serial[ii][jj];
				out_data[max_length * (1 + 2 * jj)] = KeyPointHeight_serial[ii][jj];
				for (int pp = 0; pp < KeyPoint_serial[ii][jj].size(); pp++)
				{
					out_data[pp + 1 + max_length * 2 * jj] = KeyPoint_serial[ii][jj][pp].x;
					out_data[pp + 1 + max_length * (1 + 2 * jj)] = KeyPoint_serial[ii][jj][pp].y;
				}
				for (int pp = KeyPoint_serial[ii][jj].size(); pp < max_length - 1; pp++)
				{
					out_data[pp + 1 + max_length * 2 * jj] = NAN;
					out_data[pp + 1 + max_length * (1 + 2 * jj)] = NAN;
				}
			}
			else
			{
				out_data[0 + max_length * 2 * jj] = 0;
				out_data[0 + max_length * (1 + 2 * jj)] = 0;
				for (int pp = 0; pp < max_length - 1; pp++)
				{
					out_data[pp + 1 + max_length * 2 * jj] = NAN;
					out_data[pp + 1 + max_length * (1 + 2 * jj)] = NAN;
				}
			}
		}
		memcpy((void*)(mxGetPr(pMxArray)), (void*)out_data, sizeof(double)* Image_serial_name[ii].size() * 2 * max_length);
		QString var_mat_name = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) 
			+ tr("_Group_") + QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 8, 10, QLatin1Char('0')) ;
		matPutVariable(pmatFile, var_mat_name.toLocal8Bit().toStdString().data(), pMxArray);
		delete[] out_data;
		mxDestroyArray(pMxArray);

	}
	QMessageBox::information(this, tr("Save KeyPoint File"), tr("Save Successfully."), QMessageBox::Ok);
	matClose(pmatFile);
}

void Camera_calibration_module::save_CalibrationeResult_file(QString path_save)
{
	FILE* f = fopen(path_save.toLocal8Bit().data(), "wb");
	unsigned char save_mode = 2;
	switch (cur_target_type_cal)
	{
	case Chess_Board_type:
		save_mode = 2;
		break;
	case Circle_Board_type:
		save_mode = 4;
		break;
	case Ori_Circle_Board_type:
		save_mode = 6;
		break;
	case Speckle_Board_type:
		save_mode = 8;
		break;
	default:
		save_mode = 2;
		break;
	}
	size_t return_size = fwrite(&save_mode, sizeof(unsigned char), 1, f);
	if (return_size != 1)
	{
		QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		fclose(f);
		return;
	}
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		int has_found = -1;
		for (int ss = 0; ss < Result_index.size(); ss++)
		{
			if (Result_index[ss] == ii)
			{
				has_found = ss;
				break;
			}
		}
		if (has_found == -1)
		{
			continue;
		}
		int camera_index = ii;
		int group_index = Camera_combobox[ii]->currentIndex();
		return_size = fwrite(&group_index, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		return_size = fwrite(&camera_index, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		return_size = fwrite(Result_K[has_found], sizeof(double), 5, f);
		if (return_size != 5)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		return_size = fwrite(Result_Dis_K[has_found], sizeof(double), 6, f);
		if (return_size != 6)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		return_size = fwrite(Result_Dis_P[has_found], sizeof(double), 2, f);
		if (return_size != 2)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		return_size = fwrite(Result_Dis_T[has_found], sizeof(double), 4, f);
		if (return_size != 4)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		double* R_temp = new double[4];
		R_temp[0] = Result_R_overall[has_found].x();
		R_temp[1] = Result_R_overall[has_found].y();
		R_temp[2] = Result_R_overall[has_found].z();
		R_temp[3] = Result_R_overall[has_found].w();
		return_size = fwrite(R_temp, sizeof(double), 4, f);
		if (return_size != 4)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		delete[] R_temp;

		double* T_temp = new double[3];
		T_temp[0] = Result_T_overall[has_found].x();
		T_temp[1] = Result_T_overall[has_found].y();
		T_temp[2] = Result_T_overall[has_found].z();
		return_size = fwrite(T_temp, sizeof(double), 3, f);
		if (return_size != 3)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		delete[] T_temp;

		int verify_size = Result_K_verify[has_found].size();
		return_size = fwrite(&verify_size, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		for (int jj = 0; jj < verify_size; jj++)
		{
			return_size = fwrite(Result_K_verify[has_found][jj], sizeof(double), 5, f);
			if (return_size != 5)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}

			return_size = fwrite(Result_Dis_K_verify[has_found][jj], sizeof(double), 6, f);
			if (return_size != 6)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}

			return_size = fwrite(Result_Dis_P_verify[has_found][jj], sizeof(double), 2, f);
			if (return_size != 2)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}

			return_size = fwrite(Result_Dis_T_verify[has_found][jj], sizeof(double), 4, f);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			double* R_temp_verify = new double[4];
			R_temp_verify[0] = Result_R_overall_verify[has_found][jj].x();
			R_temp_verify[1] = Result_R_overall_verify[has_found][jj].y();
			R_temp_verify[2] = Result_R_overall_verify[has_found][jj].z();
			R_temp_verify[3] = Result_R_overall_verify[has_found][jj].w();
			return_size = fwrite(R_temp_verify, sizeof(double), 4, f);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			delete[] R_temp_verify;

			double* T_temp_verify = new double[3];
			T_temp_verify[0] = Result_T_overall_verify[has_found][jj].x();
			T_temp_verify[1] = Result_T_overall_verify[has_found][jj].y();
			T_temp_verify[2] = Result_T_overall_verify[has_found][jj].z();
			return_size = fwrite(T_temp_verify, sizeof(double), 3, f);
			if (return_size != 3)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			delete[] T_temp_verify;

			int corn_opt_number = Result_update_calib_points[has_found].size();
			return_size = fwrite(&corn_opt_number, sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			for (int pp = 0; pp < corn_opt_number; pp++)
			{
				double x_temp = Result_update_calib_points_verify[has_found][jj][pp].x;
				double y_temp = Result_update_calib_points_verify[has_found][jj][pp].y;
				double z_temp = Result_update_calib_points_verify[has_found][jj][pp].z;
				return_size = fwrite(&x_temp, sizeof(double), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&y_temp, sizeof(double), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&z_temp, sizeof(double), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
			}

		}

		return_size = fwrite(&Result_re_err[has_found].at(Result_re_err[has_found].size() - 1), sizeof(double), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		int itera_size_single = Result_costdata_single[has_found].size();
		return_size = fwrite(&itera_size_single, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		for (int jj = 0; jj < itera_size_single; jj++)
		{
			return_size = fwrite(&Result_costdata_single[has_found][jj], sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
		}

		int itera_size_all = Result_costdata_all[has_found].size();
		return_size = fwrite(&itera_size_all, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		for (int jj = 0; jj < itera_size_all; jj++)
		{
			return_size = fwrite(&Result_costdata_all[has_found][jj], sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
		}

		int corn_opt_number = Result_update_calib_points[has_found].size();
		return_size = fwrite(&corn_opt_number, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}
		for (int jj = 0; jj < corn_opt_number; jj++)
		{
			double x_temp = Result_update_calib_points[has_found][jj].x;
			double y_temp = Result_update_calib_points[has_found][jj].y;
			double z_temp = Result_update_calib_points[has_found][jj].z;
			return_size = fwrite(&x_temp, sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&y_temp, sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&z_temp, sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
		}
		int image_size = Image_serial_name[ii].size();
		return_size = fwrite(&image_size, sizeof(int), 1, f);
		if (return_size != 1)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			fclose(f);
			return;
		}

		for (int jj = 0; jj < Image_serial_name[ii].size(); jj++)
		{
			int image_index = jj;

			return_size = fwrite(&image_index, sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			bool is_enable_every = Result_isenable[has_found][jj];
			return_size = fwrite(&is_enable_every, sizeof(bool), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}

			double reproj_err_every = Result_re_err[has_found][jj];
			return_size = fwrite(&reproj_err_every, sizeof(double), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}

			double* R_temp_every = new double[4];
			R_temp_every[0] = Result_R[has_found][jj].x();
			R_temp_every[1] = Result_R[has_found][jj].y();
			R_temp_every[2] = Result_R[has_found][jj].z();
			R_temp_every[3] = Result_R[has_found][jj].w();
			return_size = fwrite(R_temp_every, sizeof(double), 4, f);
			if (return_size != 4)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			delete[] R_temp_every;

			double* T_temp_every = new double[3];
			T_temp_every[0] = Result_T[has_found][jj].x();
			T_temp_every[1] = Result_T[has_found][jj].y();
			T_temp_every[2] = Result_T[has_found][jj].z();
			return_size = fwrite(T_temp_every, sizeof(double), 3, f);
			if (return_size != 3)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			delete[] T_temp_every;

			long long string_len = Image_serial_name[ii][jj].toLocal8Bit().toStdString().size();
			return_size = fwrite(&string_len, sizeof(long long), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			std::string im_path = Image_serial_name[ii][jj].toLocal8Bit().toStdString();
			return_size = fwrite(&im_path[0], sizeof(char), string_len, f);
			if (return_size != string_len)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&ImageWidth_serial[ii][jj], sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			return_size = fwrite(&ImageHeight_serial[ii][jj], sizeof(int), 1, f);
			if (return_size != 1)
			{
				QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
				fclose(f);
				return;
			}
			int corn_number = KeyPoint_serial[ii][jj].size();
			if (corn_number == KeyPointWidth_serial[ii][jj] * KeyPointHeight_serial[ii][jj])
			{
				return_size = fwrite(&corn_number, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&KeyPointWidth_serial[ii][jj], sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&KeyPointHeight_serial[ii][jj], sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				if (cur_target_type_cal == Target_type::Ori_Circle_Board_type)
				{
					return_size = fwrite(&cur_target_offset_for_ori_wid, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_offset_for_ori_hei, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_incre_for_ori_wid, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&cur_target_incre_for_ori_hei, sizeof(int), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
				}

				for (int pp = 0; pp < corn_number; pp++)
				{
					double corn_x = KeyPoint_serial[ii][jj][pp].x;
					double corn_y = KeyPoint_serial[ii][jj][pp].y;
					return_size = fwrite(&corn_x, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&corn_y, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					double err_x, err_y;
					if (Result_isenable[has_found][jj])
					{
						err_x = Result_Re_map[has_found][jj][pp].x;
						err_y = Result_Re_map[has_found][jj][pp].y;
					}
					else
					{
						err_x = std::numeric_limits<double>::max();
						err_y = std::numeric_limits<double>::max();
					}
					return_size = fwrite(&err_x, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
					return_size = fwrite(&err_y, sizeof(double), 1, f);
					if (return_size != 1)
					{
						QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
						fclose(f);
						return;
					}
				}
			}
			else
			{
				corn_number = 0;
				int corn_w = 0;
				int corn_h = 0;
				return_size = fwrite(&corn_number, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&corn_w, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
				return_size = fwrite(&corn_h, sizeof(int), 1, f);
				if (return_size != 1)
				{
					QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
					fclose(f);
					return;
				}
			}
		}
	}
	QMessageBox::information(this, tr("Save Result File"), tr("Save Successfully."), QMessageBox::Ok);
	fclose(f);
}
void Camera_calibration_module::save_CalibrationeResult_file_csv(QString path_save)
{
	QFile file(path_save);
	if (!file.exists())  //文件不存在的时新建
	{
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream txtOutPut(&file);
		file.close();
	}
	file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	QTextStream txtOutPut(&file);
	txtOutPut.setEncoding(QStringConverter::System);
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		int has_found = -1;
		for (int ss = 0; ss < Result_index.size(); ss++)
		{
			if (Result_index[ss] == ii)
			{
				has_found = ss;
				break;
			}
		}
		if (has_found == -1)
		{
			continue;
		}
		txtOutPut << tr("Camera-Index-Group") << "," << tr("Camera-") << QString::number(ii + 1) << "," << tr("Group-") << QString::number(Camera_combobox[ii]->currentIndex() + 1) << "\n";
		txtOutPut << tr("IntrinsicMatrix") << "," << Result_K[has_found][0] << "," << Result_K[has_found][4] << "," << Result_K[has_found][2] << "\n";
		txtOutPut << tr("") << "," << 0 << "," << Result_K[has_found][1] << "," << Result_K[has_found][3]   << "\n";
		txtOutPut << tr("") << "," << 0 << "," << 0 << "," << 1 << "\n";
		txtOutPut << tr("RadialDistortion") << "," << Result_Dis_K[has_found][0] << "," << Result_Dis_K[has_found][1] << "," << Result_Dis_K[has_found][2] << ","
			<< Result_Dis_K[has_found][3] << "," << Result_Dis_K[has_found][4] << "," << Result_Dis_K[has_found][5]
			<< "\n";
		txtOutPut << tr("TangentialDistortion") << "," << Result_Dis_P[has_found][0] << "," << Result_Dis_P[has_found][1] << "\n";
		txtOutPut << tr("ThinPrismDistortion") << "," << Result_Dis_T[has_found][0] << "," << Result_Dis_T[has_found][1] << "," << Result_Dis_T[has_found][2] << "," << Result_Dis_T[has_found][3] << "\n";
		txtOutPut << tr("Rotation(Quaternion)") << "," << Result_R_overall[has_found].x() << "," 
			<< Result_R_overall[has_found].y() << "," << Result_R_overall[has_found].z() << "," 
			<< Result_R_overall[has_found].w() << "\n";
		txtOutPut << tr("Translation") << "," << Result_T_overall[has_found].x() << ","
			<< Result_T_overall[has_found].y() << "," << Result_T_overall[has_found].z() << "\n";
		txtOutPut << tr("Stability verification parameters") << "," << tr("Fx") << "," << tr("Fy") << "," << tr("Cx") << "," << tr("Cy") << ","
			<< tr("Skew") << ","
			<< tr("RadialDistortion-1") << "," << tr("RadialDistortion-2") << "," << tr("RadialDistortion-3") << ","
			<< tr("RadialDistortion-4") << "," << tr("RadialDistortion-5") << "," << tr("RadialDistortion-6") << ","
			<< tr("TangentialDistortion-1") << "," << tr("TangentialDistortion-2") << ","
			<< tr("ThinPrismDistortion-1") << "," << tr("ThinPrismDistortion-2") << "," << tr("ThinPrismDistortion-3") << "," << tr("ThinPrismDistortion-4") << ","
			<< tr("Rotation(Quaternion)-X") << "," << tr("Rotation(Quaternion)-Y") << "," << tr("Rotation(Quaternion)-Z") << "," << tr("Rotation(Quaternion)-W") << ","
			<< tr("Translation-X") << "," << tr("Translation-Y") << "," << tr("Translation-Z") << "\n";
		if (has_found < Result_K_verify.size())
		{
			for (int jj = 0; jj < Result_K_verify[has_found].size(); jj++)
			{
				txtOutPut << tr("") << "," << Result_K_verify[has_found][jj][0] << "," << Result_K_verify[has_found][jj][1] << "," << Result_K_verify[has_found][jj][2] << "," << Result_K_verify[has_found][jj][3] << ","
					<< Result_K_verify[has_found][jj][4] << ","
					<< Result_Dis_K_verify[has_found][jj][0] << "," << Result_Dis_K_verify[has_found][jj][1] << "," << Result_Dis_K_verify[has_found][jj][2] << ","
					<< Result_Dis_K_verify[has_found][jj][3] << "," << Result_Dis_K_verify[has_found][jj][4] << "," << Result_Dis_K_verify[has_found][jj][5] << ","
					<< Result_Dis_P_verify[has_found][jj][0] << "," << Result_Dis_P_verify[has_found][jj][1] << ","
					<< Result_Dis_T_verify[has_found][jj][0] << "," << Result_Dis_T_verify[has_found][jj][1] << "," << Result_Dis_T_verify[has_found][jj][2] << "," << Result_Dis_T_verify[has_found][jj][3] << ","
					<< Result_R_overall_verify[has_found][jj].x() << "," << Result_R_overall_verify[has_found][jj].y() << "," << Result_R_overall_verify[has_found][jj].z() << "," << Result_R_overall_verify[has_found][jj].w() << ","
					<< Result_T_overall_verify[has_found][jj].x() << "," << Result_T_overall_verify[has_found][jj].y() << "," << Result_T_overall_verify[has_found][jj].z() << "\n";
			}
		}
		txtOutPut << tr("Re-projection Error") << "," << Result_re_err[has_found].at(Result_re_err[has_found].size() - 1) << "\n";
		txtOutPut << tr("Single Camera Iteration Cost");
		for (int jj = 0; jj < Result_costdata_single[has_found].size(); jj++)
		{
			txtOutPut << "," << Result_costdata_single[has_found][jj];
		}
		txtOutPut << "\n";
		txtOutPut << tr("Multi Cameras Iteration Cost");
		for (int jj = 0; jj < Result_costdata_all[has_found].size(); jj++)
		{
			txtOutPut << "," << Result_costdata_all[has_found][jj];
		}
		txtOutPut << "\n";
		txtOutPut << tr("(Corrected) Calibrator Morphology") << "\n";
		txtOutPut << tr("") << "," << tr("X");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "," << Result_update_calib_points[has_found][jj].x;
		}
		txtOutPut << "\n";
		txtOutPut << tr("") << "," << tr("Y");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "," << Result_update_calib_points[has_found][jj].y;
		}
		txtOutPut << "\n";
		txtOutPut << tr("") << "," << tr("Z");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "," << Result_update_calib_points[has_found][jj].z;
		}
		txtOutPut << "\n";
		for (int jj = 0; jj < Image_serial_name[has_found].size(); jj++)
		{
			txtOutPut << Image_serial_name[has_found][jj].toLocal8Bit() << ",";
			txtOutPut << ImageWidth_serial[has_found][jj] << ",";
			txtOutPut << ImageHeight_serial[has_found][jj] << ",";
			int corn_number = KeyPoint_serial[has_found][jj].size();
			if (corn_number == KeyPointWidth_serial[has_found][jj] * KeyPointHeight_serial[has_found][jj])
			{
				txtOutPut << corn_number << ",";
				txtOutPut << KeyPointWidth_serial[has_found][jj] << ",";
				txtOutPut << KeyPointHeight_serial[has_found][jj] << "\n";
				txtOutPut << "" << "," << "X" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].x << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].x << ",";
					}
				}
				txtOutPut << "" << "," << "Y" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].y << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].y << ",";
					}
				}
				txtOutPut << "" << "," << "Error_x" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp < Result_Re_map[ii][jj].size())
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << Result_Re_map[ii][jj][pp].x << "\n";
						}
						else
						{
							txtOutPut << Result_Re_map[ii][jj][pp].x << ",";
						}
					}
					else
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << "NAN" << ",";
						}
						else
						{
							txtOutPut << "NAN" << "\n";
						}
					}
				}
				txtOutPut << "" << "," << "Error_y" << ",";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp < Result_Re_map[ii][jj].size())
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << Result_Re_map[ii][jj][pp].y << "\n";
						}
						else
						{
							txtOutPut << Result_Re_map[ii][jj][pp].y << ",";
						}
					}
					else
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << "NAN" << ",";
						}
						else
						{
							txtOutPut << "NAN" << "\n";
						}
					}
				}
			}
			else
			{
				txtOutPut << 0 << ",";
				txtOutPut << 0 << ",";
				txtOutPut << 0 << "\n";
				txtOutPut << "\n";
			}
		}
		txtOutPut << "\n";
	}
	QMessageBox::information(this, tr("Save Result File"), tr("Save Successfully."), QMessageBox::Ok);
	file.close();
}
void Camera_calibration_module::save_CalibrationeResult_file_txt(QString path_save)
{
	QFile file(path_save);
	if (!file.exists())  //文件不存在的时新建
	{
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream txtOutPut(&file);
		file.close();
	}
	file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
	QTextStream txtOutPut(&file);
	txtOutPut.setEncoding(QStringConverter::System);
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		int has_found = -1;
		for (int ss = 0; ss < Result_index.size(); ss++)
		{
			if (Result_index[ss] == ii)
			{
				has_found = ss;
				break;
			}
		}
		if (has_found == -1)
		{
			continue;
		}
		txtOutPut << tr("Camera-Index-Group") << "\t" << tr("Camera-") << QString::number(ii + 1) << "\t" << tr("Group-") << QString::number(Camera_combobox[ii]->currentIndex() + 1) << "\n";
		txtOutPut << tr("IntrinsicMatrix") << "\t" << Result_K[has_found][0] << "\t" << Result_K[has_found][4] << "\t" << Result_K[has_found][2] << "\n";
		txtOutPut << tr("") << "\t" << 0 << "\t" << Result_K[has_found][1] << "\t" << Result_K[has_found][3]<< "\n";
		txtOutPut << tr("") << "\t" << 0 << "\t" << 0 << "\t" << 1 << "\n";
		txtOutPut << tr("RadialDistortion") << "\t" << Result_Dis_K[has_found][0] << "\t" << Result_Dis_K[has_found][1] << "\t" << Result_Dis_K[has_found][2] << "\t"
			<< Result_Dis_K[has_found][3] << "\t" << Result_Dis_K[has_found][4] << "\t" << Result_Dis_K[has_found][5]
			<< "\n";
		txtOutPut << tr("TangentialDistortion") << "\t" << Result_Dis_P[has_found][0] << "\t" << Result_Dis_P[has_found][1] << "\n";
		txtOutPut << tr("ThinPrismDistortion") << "\t" << Result_Dis_T[has_found][0] << "\t" << Result_Dis_T[has_found][1] << "\t" << Result_Dis_T[has_found][2] << "\t" << Result_Dis_T[has_found][3] << "\n";
		txtOutPut << tr("Rotation(Quaternion)") << "\t" << Result_R_overall[has_found].x() << "\t"
			<< Result_R_overall[has_found].y() << "\t" << Result_R_overall[has_found].z() << "\t"
			<< Result_R_overall[has_found].w() << "\n";
		txtOutPut << tr("Translation") << "\t" << Result_T_overall[has_found].x() << "\t"
			<< Result_T_overall[has_found].y() << "\t" << Result_T_overall[has_found].z() << "\n";
		txtOutPut << tr("Stability verification parameters") << "\t" << tr("Fx") << "\t" << tr("Fy") << "\t" << tr("Cx") << "\t" << tr("Cy") << "\t"
			<< tr("Skew") << "\t"
			<< tr("RadialDistortion-1") << "\t" << tr("RadialDistortion-2") << "\t" << tr("RadialDistortion-3") << "\t"
			<< tr("RadialDistortion-4") << "\t" << tr("RadialDistortion-5") << "\t" << tr("RadialDistortion-6") << "\t"
			<< tr("TangentialDistortion-1") << "\t" << tr("TangentialDistortion-2") << "\t"
			<< tr("ThinPrismDistortion-1") << "\t" << tr("ThinPrismDistortion-2") << "\t" << tr("ThinPrismDistortion-3") << "\t" << tr("ThinPrismDistortion-4") << "\t"
			<< tr("Rotation(Quaternion)-X") << "\t" << tr("Rotation(Quaternion)-Y") << "\t" << tr("Rotation(Quaternion)-Z") << "\t" << tr("Rotation(Quaternion)-W") << "\t"
			<< tr("Translation-X") << "\t" << tr("Translation-Y") << "\t" << tr("Translation-Z") << "\n";
		if (has_found < Result_K_verify.size())
		{
			for (int jj = 0; jj < Result_K_verify[has_found].size(); jj++)
			{
				txtOutPut << tr("") << "\t" << Result_K_verify[has_found][jj][0] << "\t" << Result_K_verify[has_found][jj][1] << "\t" << Result_K_verify[has_found][jj][2] << "\t" << Result_K_verify[has_found][jj][3] << "\t"
					<< Result_K_verify[has_found][jj][4] << "\t"
					<< Result_Dis_K_verify[has_found][jj][0] << "\t" << Result_Dis_K_verify[has_found][jj][1] << "\t" << Result_Dis_K_verify[has_found][jj][2] << "\t"
					<< Result_Dis_K_verify[has_found][jj][3] << "\t" << Result_Dis_K_verify[has_found][jj][4] << "\t" << Result_Dis_K_verify[has_found][jj][5] << "\t"
					<< Result_Dis_P_verify[has_found][jj][0] << "\t" << Result_Dis_P_verify[has_found][jj][1] << "\t"
					<< Result_Dis_T_verify[has_found][jj][0] << "\t" << Result_Dis_T_verify[has_found][jj][1] << "\t" << Result_Dis_T_verify[has_found][jj][2] << "\t" << Result_Dis_T_verify[has_found][jj][3] << "\t"
					<< Result_R_overall_verify[has_found][jj].x() << "\t" << Result_R_overall_verify[has_found][jj].y() << "\t" << Result_R_overall_verify[has_found][jj].z() << "\t" << Result_R_overall_verify[has_found][jj].w() << "\t"
					<< Result_T_overall_verify[has_found][jj].x() << "\t" << Result_T_overall_verify[has_found][jj].y() << "\t" << Result_T_overall_verify[has_found][jj].z() << "\n";
			}
		}
		txtOutPut << tr("Re-projection Error") << "\t" << Result_re_err[has_found].at(Result_re_err[has_found].size() - 1) << "\n";
		txtOutPut << tr("Single Camera Iteration Cost");
		for (int jj = 0; jj < Result_costdata_single[has_found].size(); jj++)
		{
			txtOutPut << "\t" << Result_costdata_single[has_found][jj];
		}
		txtOutPut << "\n";
		txtOutPut << tr("Multi Cameras Iteration Cost");
		for (int jj = 0; jj < Result_costdata_all[has_found].size(); jj++)
		{
			txtOutPut << "\t" << Result_costdata_all[has_found][jj];
		}
		txtOutPut << "\n";
		txtOutPut << tr("(Corrected) Calibrator Morphology") << "\n";
		txtOutPut << tr("") << "\t" << tr("X");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "\t" << Result_update_calib_points[has_found][jj].x;
		}
		txtOutPut << "\n";
		txtOutPut << tr("") << "\t" << tr("Y");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "\t" << Result_update_calib_points[has_found][jj].y;
		}
		txtOutPut << "\n";
		txtOutPut << tr("") << "\t" << tr("Z");
		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			txtOutPut << "\t" << Result_update_calib_points[has_found][jj].z;
		}
		txtOutPut << "\n";
		for (int jj = 0; jj < Image_serial_name[has_found].size(); jj++)
		{
			txtOutPut << Image_serial_name[has_found][jj].toLocal8Bit() << "\t";
			txtOutPut << ImageWidth_serial[has_found][jj] << "\t";
			txtOutPut << ImageHeight_serial[has_found][jj] << "\t";
			int corn_number = KeyPoint_serial[has_found][jj].size();
			if (corn_number == KeyPointWidth_serial[has_found][jj] * KeyPointHeight_serial[has_found][jj])
			{
				txtOutPut << corn_number << "\t";
				txtOutPut << KeyPointWidth_serial[has_found][jj] << "\t";
				txtOutPut << KeyPointHeight_serial[has_found][jj] << "\n";
				txtOutPut << "" << "\t" << "X" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].x << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].x << "\t";
					}
				}
				txtOutPut << "" << "\t" << "Y" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp == (corn_number - 1))
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].y << "\n";
					}
					else
					{
						txtOutPut << KeyPoint_serial[has_found][jj][pp].y << "\t";
					}
				}
				txtOutPut << "" << "\t" << "Error_x" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp < Result_Re_map[ii][jj].size())
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << Result_Re_map[ii][jj][pp].x << "\n";
						}
						else
						{
							txtOutPut << Result_Re_map[ii][jj][pp].x << "\t";
						}
					}
					else
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << "NAN" << "\t";
						}
						else
						{
							txtOutPut << "NAN" << "\n";
						}
					}
				}
				txtOutPut << "" << "\t" << "Error_y" << "\t";
				for (int pp = 0; pp < corn_number; pp++)
				{
					if (pp < Result_Re_map[ii][jj].size())
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << Result_Re_map[ii][jj][pp].y << "\n";
						}
						else
						{
							txtOutPut << Result_Re_map[ii][jj][pp].y << "\t";
						}
					}
					else
					{
						if (pp == (corn_number - 1))
						{
							txtOutPut << "NAN" << "\t";
						}
						else
						{
							txtOutPut << "NAN" << "\n";
						}
					}
				}
			}
			else
			{
				txtOutPut << 0 << "\t";
				txtOutPut << 0 << "\t";
				txtOutPut << 0 << "\n";
				txtOutPut << "\n";
			}
		}
		txtOutPut << "\n";
	}
	QMessageBox::information(this, tr("Save Result File"), tr("Save Successfully."), QMessageBox::Ok);
	file.close();
}
void Camera_calibration_module::save_CalibrationeResult_file_mat(QString path_save)
{
	MATFile* pmatFile = NULL;
	pmatFile = matOpen(path_save.toLocal8Bit().data(), "w");
	if (pmatFile == NULL)
	{
		QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
		return;
	}
	for (int ii = 0; ii < MAX_CALIBRAT_CAMERA; ii++)
	{
		int has_found = -1;
		for (int ss = 0; ss < Result_index.size(); ss++)
		{
			if (Result_index[ss] == ii)
			{
				has_found = ss;
				break;
			}
		}
		if (has_found == -1)
		{
			continue;
		}
		mxArray* pMxArray_intrinsic = NULL;
		pMxArray_intrinsic = mxCreateDoubleMatrix(3, 3, mxREAL);
		if (pMxArray_intrinsic == NULL)
		{
			QMessageBox::warning(this, tr("Save KeyPoint File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_intrinsic = new double[9];
		out_data_intrinsic[0] = Result_K[has_found][0];
		out_data_intrinsic[3] = Result_K[has_found][4];
		out_data_intrinsic[6] = Result_K[has_found][2];
		out_data_intrinsic[4] = Result_K[has_found][1];
		out_data_intrinsic[7] = Result_K[has_found][3];
		out_data_intrinsic[1] = 0;
		out_data_intrinsic[2] = 0;
		out_data_intrinsic[5] = 0;
		out_data_intrinsic[8] = 1;
		memcpy((void*)(mxGetPr(pMxArray_intrinsic)), (void*)out_data_intrinsic, sizeof(double) * 9);
		QString var_mat_name_intrinsic = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_") 
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+tr("_IntrinsicMatrix");
		matPutVariable(pmatFile, var_mat_name_intrinsic.toLocal8Bit().toStdString().data(), pMxArray_intrinsic);
		delete[] out_data_intrinsic;
		mxDestroyArray(pMxArray_intrinsic);
		mxArray* pMxArray_radial = NULL;
		pMxArray_radial = mxCreateDoubleMatrix(1, 6, mxREAL);
		if (pMxArray_radial == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_radial = new double[6];
		out_data_radial[0] = Result_Dis_K[has_found][0];
		out_data_radial[1] = Result_Dis_K[has_found][1];
		out_data_radial[2] = Result_Dis_K[has_found][2];
		out_data_radial[3] = Result_Dis_K[has_found][3];
		out_data_radial[4] = Result_Dis_K[has_found][4];
		out_data_radial[5] = Result_Dis_K[has_found][5];
		memcpy((void*)(mxGetPr(pMxArray_radial)), (void*)out_data_radial, sizeof(double) * 6);
		QString var_mat_name_radial = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_RadialDistortion");
		matPutVariable(pmatFile, var_mat_name_radial.toLocal8Bit().toStdString().data(), pMxArray_radial);
		delete[] out_data_radial;
		mxDestroyArray(pMxArray_radial);
		mxArray* pMxArray_tangential = NULL;
		pMxArray_tangential = mxCreateDoubleMatrix(1, 2, mxREAL);
		if (pMxArray_tangential == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_tangential = new double[2];
		out_data_tangential[0] = Result_Dis_P[has_found][0];
		out_data_tangential[1] = Result_Dis_P[has_found][1];
		memcpy((void*)(mxGetPr(pMxArray_tangential)), (void*)out_data_tangential, sizeof(double) * 2);
		QString var_mat_name_tangential = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_TangentialDistortion");
		matPutVariable(pmatFile, var_mat_name_tangential.toLocal8Bit().toStdString().data(), pMxArray_tangential);
		delete[] out_data_tangential;
		mxDestroyArray(pMxArray_tangential);
		mxArray* pMxArray_thinprism = NULL;
		pMxArray_thinprism = mxCreateDoubleMatrix(1, 4, mxREAL);
		if (pMxArray_thinprism == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_thinprism = new double[4];
		out_data_thinprism[0] = Result_Dis_T[has_found][0];
		out_data_thinprism[1] = Result_Dis_T[has_found][1];
		out_data_thinprism[2] = Result_Dis_T[has_found][2];
		out_data_thinprism[3] = Result_Dis_T[has_found][3];
		memcpy((void*)(mxGetPr(pMxArray_thinprism)), (void*)out_data_thinprism, sizeof(double) * 4);
		QString var_mat_name_thinprism = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_ThinPrismDistortion");
		matPutVariable(pmatFile, var_mat_name_thinprism.toLocal8Bit().toStdString().data(), pMxArray_thinprism);
		delete[] out_data_thinprism;
		mxDestroyArray(pMxArray_thinprism);
		mxArray* pMxArray_overall_R = NULL;
		pMxArray_overall_R = mxCreateDoubleMatrix(3, 3, mxREAL);
		if (pMxArray_overall_R == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		Eigen::Matrix3d R_odom_curr_tmp = Result_R_overall[has_found].matrix();
		double* out_data_overall_R = new double[9];
		out_data_overall_R[0] = R_odom_curr_tmp(0, 0);
		out_data_overall_R[1] = R_odom_curr_tmp(1, 0);
		out_data_overall_R[2] = R_odom_curr_tmp(2, 0);
		out_data_overall_R[3] = R_odom_curr_tmp(0, 1);
		out_data_overall_R[4] = R_odom_curr_tmp(1, 1);
		out_data_overall_R[5] = R_odom_curr_tmp(2, 1);
		out_data_overall_R[6] = R_odom_curr_tmp(0, 2);
		out_data_overall_R[7] = R_odom_curr_tmp(1, 2);
		out_data_overall_R[8] = R_odom_curr_tmp(2, 2);
		memcpy((void*)(mxGetPr(pMxArray_overall_R)), (void*)out_data_overall_R, sizeof(double) * 9);
		QString var_mat_name_overall_R = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_R");
		matPutVariable(pmatFile, var_mat_name_overall_R.toLocal8Bit().toStdString().data(), pMxArray_overall_R);
		delete[] out_data_overall_R;
		mxDestroyArray(pMxArray_overall_R);
		mxArray* pMxArray_overall_T = NULL;
		pMxArray_overall_T = mxCreateDoubleMatrix(1, 3, mxREAL);
		if (pMxArray_overall_T == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_overall_T = new double[3];
		out_data_overall_T[0] = Result_T_overall[has_found].x();
		out_data_overall_T[1] = Result_T_overall[has_found].y();
		out_data_overall_T[2] = Result_T_overall[has_found].z();
		memcpy((void*)(mxGetPr(pMxArray_overall_T)), (void*)out_data_overall_T, sizeof(double) * 3);
		QString var_mat_name_overall_T = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_T");
		matPutVariable(pmatFile, var_mat_name_overall_T.toLocal8Bit().toStdString().data(), pMxArray_overall_T);
		delete[] out_data_overall_T;
		mxDestroyArray(pMxArray_overall_T);
		mxArray* pMxArray_calibrator = NULL;
		pMxArray_calibrator = mxCreateDoubleMatrix(Result_update_calib_points[has_found].size(), 3, mxREAL);
		if (pMxArray_calibrator == NULL)
		{
			QMessageBox::warning(this, tr("Save Result File"), tr("Save Failed!"), QMessageBox::Ok, QMessageBox::Ok);
			return;
		}
		double* out_data_calibrator = new double[Result_update_calib_points[has_found].size() * 3];

		for (int jj = 0; jj < Result_update_calib_points[has_found].size(); jj++)
		{
			out_data_calibrator[jj] = Result_update_calib_points[has_found][jj].x;
			out_data_calibrator[jj + Result_update_calib_points[has_found].size()] = Result_update_calib_points[has_found][jj].y;
			out_data_calibrator[jj + 2 * Result_update_calib_points[has_found].size()] = Result_update_calib_points[has_found][jj].z;
		}
		memcpy((void*)(mxGetPr(pMxArray_calibrator)), (void*)out_data_calibrator, sizeof(double) * Result_update_calib_points[has_found].size() * 3);
		QString var_mat_name_calibrator = tr("Camera_") + QString("%1").arg(ii + 1, 4, 10, QLatin1Char('0')) + tr("_Group_")
			+ QString("%1").arg(Camera_combobox[ii]->currentIndex() + 1, 4, 10, QLatin1Char('0'))
			+ tr("_Calibrator");
		matPutVariable(pmatFile, var_mat_name_calibrator.toLocal8Bit().toStdString().data(), pMxArray_calibrator);
		delete[] out_data_calibrator;
		mxDestroyArray(pMxArray_calibrator);
	}
	QMessageBox::information(this, tr("Save Result File"), tr("Save Successfully."), QMessageBox::Ok);
	matClose(pmatFile);
}

calculate_camera_parame_thread::calculate_camera_parame_thread(
	int image_widths,
	int image_heights,
	std::vector<std::vector<cv::Point2f>> img_pts,
	std::vector<std::vector<cv::Point2f>> board_pts, std::vector<std::vector<cv::Point3f>> obj_pts
	, ceres::Solver::Summary* summarys
	, unsigned int max_iter_nums
	, double stop_values
	, unsigned int num_threadss
	, unsigned int timeouts
	, unsigned char Loss_types
	, double Loss_values
	, unsigned int Dis_K_nums
	, unsigned int Dis_P_nums
	, unsigned int Dis_T_nums
	, bool Use_shears
	, bool Use_same_Fs)
{
	image_width = image_widths;
	image_height = image_heights;
	img_pt = img_pts;
	board_pt = board_pts;
	obj_pt = obj_pts;
	summary = summarys;
	max_iter_num = max_iter_nums;
	stop_value = stop_values;
	num_threads = num_threadss;
	timeout = timeouts;
	Loss_type = Loss_types;
	Loss_value = Loss_values;
	Dis_K_num = Dis_K_nums;
	Dis_P_num = Dis_P_nums;
	Dis_T_num = Dis_T_nums;
	Use_shear = Use_shears;
	Use_same_F = Use_same_Fs;
}
void calculate_camera_parame_thread::run()
{
	CameraCalibrate_Ceres::calculate_camera_parame(
		image_width, image_height,
		obj_pt, img_pt, board_pt,
		re_err, R, T, Re_map, K, Dis_K, Dis_P, Dis_T, summary,
		max_iter_num, stop_value, num_threads, timeout, Loss_type, Loss_value,
		Dis_K_num, Dis_P_num, Dis_T_num, Use_shear, Use_same_F);
}

calculate_camera_parame_fixed_optimize_thread::calculate_camera_parame_fixed_optimize_thread(int points_nums,
	int image_widths,
	int image_heights,
	std::vector<int> fix_indexs, std::vector<int> zero_indexs,
	std::vector<std::vector<cv::Point2f>> img_pts,
	std::vector<std::vector<cv::Point2f>> board_pts, std::vector<cv::Point3f> obj_pts
	, ceres::Solver::Summary* summarys
	, unsigned int max_iter_nums
	, double stop_values
	, unsigned int num_threadss
	, unsigned int timeouts
	, unsigned char Loss_types
	, double Loss_values
	, unsigned int Dis_K_nums
	, unsigned int Dis_P_nums
	, unsigned int Dis_T_nums
	, bool Use_shears
	, bool Use_same_Fs)
{
	points_num = points_nums;
	image_width = image_widths;
	image_height = image_heights;
	fix_index = fix_indexs;
	zero_index = zero_indexs;
	img_pt = img_pts;
	board_pt = board_pts;
	obj_pt = obj_pts;
	summary = summarys;
	max_iter_num = max_iter_nums;
	stop_value = stop_values;
	num_threads = num_threadss;
	timeout = timeouts;
	Loss_type = Loss_types;
	Loss_value = Loss_values;
	Dis_K_num = Dis_K_nums;
	Dis_P_num = Dis_P_nums;
	Dis_T_num = Dis_T_nums;
	Use_shear = Use_shears;
	Use_same_F = Use_same_Fs;
}
void calculate_camera_parame_fixed_optimize_thread::run()
{
	CameraCalibrate_Ceres::calculate_camera_parame_fixed_optimize(
		points_num, image_width, image_height, fix_index, zero_index,
		img_pt, board_pt, obj_pt,
		re_err, R, T, Re_map, K, Dis_K, Dis_P, Dis_T, summary,
		max_iter_num, stop_value, num_threads, timeout, Loss_type, Loss_value,
		Dis_K_num, Dis_P_num, Dis_T_num, Use_shear, Use_same_F);
}
calculate_camera_parame_Dual_thread::calculate_camera_parame_Dual_thread(
	std::vector<std::vector<std::vector<cv::Point3f>>> obj_pts,
	std::vector<std::vector<std::vector<cv::Point2f>>> img_pts,
	std::vector<std::vector<Eigen::Quaterniond>> camQvecs
	, std::vector<std::vector<Eigen::Vector3d>> camTvecs,
	std::vector<double*> camKs,
	std::vector<double*> camDKs,
	std::vector<double*> camDPs,
	std::vector<double*> camDTs
	, ceres::Solver::Summary* summarys
	, unsigned int max_iter_nums
	, double stop_values
	, unsigned int num_threadss
	, unsigned int timeouts
	, unsigned char Loss_types
	, double Loss_values
	, unsigned int Dis_K_nums
	, unsigned int Dis_P_nums
	, unsigned int Dis_T_nums
	, bool Use_shears
	, bool Use_same_Fs)
{
	img_pt = img_pts;
	obj_pt = obj_pts;
	summary = summarys;
	max_iter_num = max_iter_nums;
	stop_value = stop_values;
	num_threads = num_threadss;
	timeout = timeouts;
	Loss_type = Loss_types;
	Loss_value = Loss_values;
	Dis_K_num = Dis_K_nums;
	Dis_P_num = Dis_P_nums;
	Dis_T_num = Dis_T_nums;
	Use_shear = Use_shears;
	Use_same_F = Use_same_Fs;
	camQvec = camQvecs;
	camTvec = camTvecs;
	camK = camKs;
	camDK = camDKs;
	camDP = camDPs;
	camDT = camDTs;
}

void calculate_camera_parame_Dual_thread::run()
{
	CameraCalibrate_Ceres::calculate_camera_parame_Dual(obj_pt, img_pt,
		reproj_err, camQvec, camTvec, Re_project_Map
		, camK, camDK, camDP, camDT,
		Dual_R, Dual_T, summary,
		max_iter_num, stop_value, num_threads, timeout, Loss_type, Loss_value,
		Dis_K_num, Dis_P_num, Dis_T_num, Use_shear, Use_same_F);
}

calculate_camera_parame_fixed_optimize_Dual_thread::calculate_camera_parame_fixed_optimize_Dual_thread(int points_nums,
	std::vector<int> fix_indexs, std::vector<int> zero_indexs,
	std::vector<std::vector<std::vector<cv::Point2f>>> img_pts,
	std::vector<cv::Point3f> obj_pts,
	std::vector<std::vector<Eigen::Quaterniond>> camQvecs
	, std::vector<std::vector<Eigen::Vector3d>> camTvecs,
	std::vector<double*> camKs,
	std::vector<double*> camDKs,
	std::vector<double*> camDPs,
	std::vector<double*> camDTs
	, ceres::Solver::Summary* summarys
	, unsigned int max_iter_nums
	, double stop_values
	, unsigned int num_threadss
	, unsigned int timeouts
	, unsigned char Loss_types
	, double Loss_values
	, unsigned int Dis_K_nums
	, unsigned int Dis_P_nums
	, unsigned int Dis_T_nums
	, bool Use_shears
	, bool Use_same_Fs)
{
	points_num = points_nums;
	fix_index = fix_indexs;
	zero_index = zero_indexs;
	img_pt = img_pts;
	obj_pt = obj_pts;
	summary = summarys;
	max_iter_num = max_iter_nums;
	stop_value = stop_values;
	num_threads = num_threadss;
	timeout = timeouts;
	Loss_type = Loss_types;
	Loss_value = Loss_values;
	Dis_K_num = Dis_K_nums;
	Dis_P_num = Dis_P_nums;
	Dis_T_num = Dis_T_nums;
	Use_shear = Use_shears;
	Use_same_F = Use_same_Fs;
	camQvec = camQvecs;
	camTvec = camTvecs;
	camK = camKs;
	camDK = camDKs;
	camDP = camDPs;
	camDT = camDTs;
}
void calculate_camera_parame_fixed_optimize_Dual_thread::run()
{
	CameraCalibrate_Ceres::calculate_camera_parame_fixed_optimize_Dual(points_num,
		fix_index, zero_index, img_pt, obj_pt,
		reproj_err, camQvec, camTvec, Re_project_Map
		, camK, camDK, camDP, camDT,
		Dual_R, Dual_T, summary, 
		max_iter_num, stop_value, num_threads, timeout, Loss_type, Loss_value,
		Dis_K_num, Dis_P_num, Dis_T_num, Use_shear, Use_same_F);
}