#include "Calibration_model_module.h"

Calibration_model_module::Calibration_model_module(Target_type target_type)
{
	ui.setupUi(this);
	switch (target_type)
	{
	case Chess_Board_type:
		break;
	case Circle_Board_type:
		break;
	case Ori_Circle_Board_type:
		break;
	case Speckle_Board_type:
		break;
	default:
		break;
	}
	connect(ui.verify_checkBox, SIGNAL(stateChanged(int)), this, SLOT(change_enable_desample()));
}

Calibration_model_module::~Calibration_model_module()
{}

void Calibration_model_module::change_enable_desample()
{
	ui.de_sample_label->setEnabled(ui.verify_checkBox->isChecked());
	ui.desample_spinBox->setEnabled(ui.verify_checkBox->isChecked());
	ui.desample_label->setEnabled(ui.verify_checkBox->isChecked());
	ui.desample_number_label->setEnabled(ui.verify_checkBox->isChecked());
}
