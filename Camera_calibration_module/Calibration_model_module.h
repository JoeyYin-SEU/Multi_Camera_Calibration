#pragma once

#include <QWidget>
#include "ui_Calibration_model_module.h"
#include "Format_Document.h"

class Calibration_model_module : public QWidget
{
	Q_OBJECT

public:
	Calibration_model_module(Target_type target_type = Chess_Board_type);
	~Calibration_model_module();
	Ui::Calibration_model_moduleClass ui;

private:

private slots:
	void change_enable_desample();

};
