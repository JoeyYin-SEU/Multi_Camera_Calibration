#include "Camera_calibration.h"

void refine_H(const std::vector<cv::Point2f>& img_pts_,
	const std::vector<cv::Point2f>& board_pts_,
	const Eigen::Matrix3d& matrix_H_,
	Eigen::Matrix3d& refined_H_)
{

	int Valid_N = 0;

	for (int i = 0; i < img_pts_.size(); ++i)
	{
		if (img_pts_[i].x != 0 && img_pts_[i].y != 0)
		{
			Valid_N++;
		}
	}

	Eigen::MatrixXd x1(2, Valid_N);
	Eigen::MatrixXd x2(2, Valid_N);

	Valid_N = 0;
	for (int i = 0; i < img_pts_.size(); ++i)
	{
		if (img_pts_[i].x != 0 && img_pts_[i].y != 0)
		{
			x1(0, Valid_N) = board_pts_[i].x;
			x1(1, Valid_N) = board_pts_[i].y;
			x2(0, Valid_N) = img_pts_[i].x;
			x2(1, Valid_N) = img_pts_[i].y;
			Valid_N++;
		}
	}


	Eigen::Matrix3d H = matrix_H_;
	//std::cout << "H:" << H<< std::endl;
	// Step 2: Refine matrix using Ceres minimizer.
	ceres::Problem problem;
	for (int i = 0; i < x1.cols(); i++)
	{
		HomographySymmetricGeometricCostFunctor
			* homography_symmetric_geometric_cost_function =
			new HomographySymmetricGeometricCostFunctor(x1.col(i),
				x2.col(i));

		problem.AddResidualBlock(
			new ceres::AutoDiffCostFunction<
			HomographySymmetricGeometricCostFunctor,
			4,  // num_residuals
			9>(homography_symmetric_geometric_cost_function),
			NULL,
			H.data());
	}
	EstimateHomographyOptions options;
	options.expected_average_symmetric_distance = 0.02;
	// Configure the solve.
	ceres::Solver::Options solver_options;
	solver_options.linear_solver_type = ceres::DENSE_QR;
	solver_options.max_num_iterations = options.max_num_iterations;
	solver_options.update_state_every_iteration = true;

	// Terminate if the average symmetric distance is good enough.
	TerminationCheckingCallback callback(x1, x2, options, &H);
	solver_options.callbacks.push_back(&callback);

	// Run the solve.
	ceres::Solver::Summary summary;
	ceres::Solve(solver_options, &problem, &summary);

	refined_H_ = H / H(2, 2);
}

void CameraCalibrate_Ceres::calculate_normalization_matrix(const std::vector<cv::Point2f>& pts, Eigen::Matrix3d& matrix_trans)
{
	double sum_x = 0, sum_y = 0;
	std::for_each(std::begin(pts), std::end(pts),
		[&](const cv::Point2f& p)
		{
			sum_x += p.x;
			sum_y += p.y;
		});
	double mean_x = sum_x / pts.size();
	double mean_y = sum_y / pts.size();

	double accmx = 0, accmy = 0;
	std::for_each(std::begin(pts), std::end(pts),
		[&](const cv::Point2f& p)
		{
			accmx += (p.x - mean_x) * (p.x - mean_x);
			accmy += (p.y - mean_y) * (p.y - mean_y);
		});
	double stdx = std::sqrt(accmx / double(pts.size() - 1));
	double stdy = std::sqrt(accmy / double(pts.size() - 1));

	double sx = std::sqrt(2.) / stdx;
	double sy = std::sqrt(2.) / stdy;

	matrix_trans << sx, 0, -sx * mean_x,
		0, sy, -sy * mean_y,
		0, 0, 1;
}

void CameraCalibrate_Ceres::estimate_H(const std::vector<cv::Point2f>& img_pts,
	const std::vector<cv::Point2f>& board_pts,
	Eigen::Matrix3d& matrix_H_)
{
	Eigen::Matrix3d matrix_normalize_img_pts;
	Eigen::Matrix3d matrix_normalize_board_pts;
	int N = img_pts.size();
	calculate_normalization_matrix(img_pts, matrix_normalize_img_pts);
	calculate_normalization_matrix(board_pts, matrix_normalize_board_pts);
	int Valid_N = 0;

	for (int i = 0; i < N; ++i)
	{
		if (img_pts[i].x != 0 && img_pts[i].y != 0)
		{
			Valid_N++;
		}
	}
	Eigen::MatrixXd M(2 * Valid_N, 9);
	M.setZero();
	Valid_N = 0;
	for (int i = 0; i < N; ++i)
	{
		if (img_pts[i].x != 0 && img_pts[i].y != 0)
		{
			Eigen::Vector3d norm_img_p = matrix_normalize_img_pts
				* Eigen::Vector3d(img_pts[i].x, img_pts[i].y, 1);
			Eigen::Vector3d norm_board_p = matrix_normalize_board_pts
				* Eigen::Vector3d(board_pts[i].x, board_pts[i].y, 1);
			//M
			M(2 * Valid_N, 0) = -norm_board_p(0);
			M(2 * Valid_N, 1) = -norm_board_p(1);
			M(2 * Valid_N, 2) = -1;
			M(2 * Valid_N, 6) = norm_img_p(0) * norm_board_p(0);
			M(2 * Valid_N, 7) = norm_img_p(0) * norm_board_p(1);
			M(2 * Valid_N, 8) = norm_img_p(0);

			M(2 * Valid_N + 1, 3) = -norm_board_p(0);
			M(2 * Valid_N + 1, 4) = -norm_board_p(1);
			M(2 * Valid_N + 1, 5) = -1;
			M(2 * Valid_N + 1, 6) = norm_img_p(1) * norm_board_p(0);
			M(2 * Valid_N + 1, 7) = norm_img_p(1) * norm_board_p(1);
			M(2 * Valid_N + 1, 8) = norm_img_p(1);
			Valid_N++;
		}

	}
	//svd solve M*h=0
	Eigen::JacobiSVD<Eigen::MatrixXd> svd(M, Eigen::ComputeFullV);
	Eigen::VectorXd V = svd.matrixV().col(8);
	matrix_H_ << V(0), V(1), V(2), V(3), V(4), V(5), V(6), V(7), V(8);
	matrix_H_ = matrix_normalize_img_pts.inverse() * matrix_H_ * matrix_normalize_board_pts;
	matrix_H_ /= matrix_H_(2, 2);
}

void CameraCalibrate_Ceres::calculate_homography(std::vector<std::vector<cv::Point2f>> board_pt, std::vector<std::vector<cv::Point2f>> img_pt, 
	std::vector<Eigen::Matrix3d>& vec_hom)
{
	vec_hom.clear();
	for (int i = 0; i < img_pt.size(); ++i)
	{
		Eigen::Matrix3d ini_H, refined_H;
		estimate_H(img_pt[i], board_pt[i], ini_H);
		refine_H(img_pt[i], board_pt[i], ini_H, refined_H);
		vec_hom.push_back(refined_H);
	}
}
void CameraCalibrate_Ceres::create_v(const Eigen::Matrix3d& h_,
	const int p,
	const int q,
	Eigen::RowVectorXd& row_v_)
{
	row_v_ << h_(0, p) * h_(0, q),
		h_(0, p)* h_(1, q) + h_(1, p) * h_(0, q),
		h_(1, p)* h_(1, q),
		h_(2, p)* h_(0, q) + h_(0, p) * h_(2, q),
		h_(2, p)* h_(1, q) + h_(1, p) * h_(2, q),
		h_(2, p)* h_(2, q);

}

void CameraCalibrate_Ceres::calculate_camera_instrinsics(const std::vector<Eigen::Matrix3d>& vec_h_, int image_width, int image_height,
	Eigen::Matrix3d& camera_matrix_)
{
	double cx = image_width / 2;
	double cy = image_height / 2;
	int numImages = vec_h_.size();

	Eigen::MatrixXd Ap(2 * numImages, 2);
	Eigen::MatrixXd bp(2 * numImages, 1);
	for (int ii = 0; ii < numImages; ii++)
	{
		Eigen::Matrix3d H = vec_h_[ii];
		H(0) = H(0) - cx * H(2);
		H(3) = H(3) - cx * H(5);
		H(6) = H(6) - cx * H(8);
		H(1) = H(1) - cy * H(2);
		H(4) = H(4) - cy * H(5);
		H(7) = H(7) - cy * H(8);
		Eigen::MatrixXd h(1, 3);
		Eigen::MatrixXd v(1, 3);
		Eigen::MatrixXd d1(1, 3);
		Eigen::MatrixXd d2(1, 3);
		Eigen::MatrixXd n(1, 4);
		h << 0, 0, 0;
		v << 0, 0, 0;
		d1 << 0, 0, 0;
		d2 << 0, 0, 0;
		n << 0, 0, 0, 0;
		for (int jj = 0; jj < 3; jj++)
		{
			double t0 = H(jj, 0);
			double t1 = H(jj, 1);
			h(jj) = t0;
			v(jj) = t1;
			d1(jj) = (t0 + t1) * 0.5;
			d2(jj) = (t0 - t1) * 0.5;
			n(0) = n(0) + t0 * t0;
			n(1) = n(1) + t1 * t1;
			n(2) = n(2) + d1(jj) * d1(jj);
			n(3) = n(3) + d2(jj) * d2(jj);
		}

		n(0) = 1.0 / sqrt(n(0));
		n(1) = 1.0 / sqrt(n(1));
		n(2) = 1.0 / sqrt(n(2));
		n(3) = 1.0 / sqrt(n(3));
		h = h * n(0);
		v = v * n(1);
		d1 = d1 * n(2);
		d2 = d2 * n(3);
		Ap(2 * ii, 0) = h(0) * v(0);
		Ap(2 * ii, 1) = h(1) * v(1);
		Ap(2 * ii + 1, 0) = d1(0) * d2(0);
		Ap(2 * ii + 1, 1) = d1(1) * d2(1);
		bp(2 * ii) = -h(2) * v(2);
		bp(2 * ii + 1) = -d1(2) * d2(2);

	}
	auto SN = Ap.transpose() * Ap;
	auto SNi = SN.inverse();
	auto f = SNi * Ap.transpose() * bp;
	double fx = sqrt(abs(1 / f(0)));
	double fy = sqrt(abs(1 / f(1)));
	camera_matrix_ << fx, 0, cx, 0, fy, cy, 0, 0, 1;
}
void CameraCalibrate_Ceres::calculate_extrinsics(const std::vector<Eigen::Matrix3d>& vec_h_,
	const Eigen::Matrix3d& camera_matrix_,
	std::vector<Eigen::MatrixXd>& vec_extrinsics_)
{
	vec_extrinsics_.clear();
	Eigen::Matrix3d inv_camera_matrix = camera_matrix_.inverse();
	for (int i = 0; i < vec_h_.size(); ++i)
	{
		Eigen::Vector3d s = inv_camera_matrix * vec_h_[i].col(0);
		double scalar_factor = 1 / s.norm();

		Eigen::Vector3d r0 = scalar_factor * inv_camera_matrix * vec_h_[i].col(0);
		Eigen::Vector3d r1 = scalar_factor * inv_camera_matrix * vec_h_[i].col(1);
		Eigen::Vector3d t = scalar_factor * inv_camera_matrix * vec_h_[i].col(2);
		Eigen::Vector3d r2 = r0.cross(r1);

		Eigen::MatrixXd RT(3, 4);
		RT.block<3, 1>(0, 0) = r0;
		RT.block<3, 1>(0, 1) = r1;
		RT.block<3, 1>(0, 2) = r2;
		RT.block<3, 1>(0, 3) = t;
		vec_extrinsics_.push_back(RT);
	}
}
void CameraCalibrate_Ceres::calculate_distortion(std::vector<std::vector<cv::Point2f>> board_pts, std::vector<std::vector<cv::Point2f>> img_pts,
	const Eigen::Matrix3d& camera_matrix_,
	const std::vector<Eigen::MatrixXd>& vec_extrinsics_,
	Eigen::VectorXd& k_)
{
	Eigen::MatrixXd D;
	Eigen::VectorXd d;
	double uc = camera_matrix_(0, 2);
	double vc = camera_matrix_(1, 2);
	for (int i = 0; i < img_pts.size(); ++i)
	{
		for (int j = 0; j < img_pts[i].size(); ++j)
		{
			Eigen::Vector4d houm_coor(board_pts[i][j].x, board_pts[i][j].y, 0, 1);
			Eigen::Vector3d uv = camera_matrix_ * vec_extrinsics_[i] * houm_coor;
			Eigen::Vector2d uv_estim(uv(0) / uv(2), uv(1) / uv(2));

			Eigen::Vector3d coor_norm = vec_extrinsics_[i] * houm_coor;
			coor_norm /= coor_norm(2);
			Eigen::Vector2d v_r(coor_norm(0), coor_norm(1));
			double r = v_r.norm();

			Eigen::RowVector2d vu((uv_estim(0) - uc) * r * r, (uv_estim(0) - uc) * r * r * r * r);
			D.conservativeResize(D.rows() + 1, 2);
			D.row(D.rows() - 1) = vu;
			Eigen::RowVector2d vv((uv_estim(1) - vc) * r * r, (uv_estim(1) - vc) * r * r * r * r);
			D.conservativeResize(D.rows() + 1, 2);
			D.row(D.rows() - 1) = vv;

			d.conservativeResize(d.size() + 1);
			d(d.size() - 1) = img_pts[i][j].x - uv_estim(0);
			d.conservativeResize(d.size() + 1);
			d(d.size() - 1) = img_pts[i][j].y - uv_estim(1);
		}
	}
	Eigen::MatrixXd DTD = D.transpose() * D;
	Eigen::MatrixXd temp = (DTD.inverse()) * D.transpose();
	k_ = temp * d;
}

double CameraCalibrate_Ceres::SymmetricGeometricDistance(const Eigen::Matrix3d& H,
	const Eigen::Vector2d& x1,
	const Eigen::Vector2d& x2)
{
	Eigen::Vector2d forward_error, backward_error;
	SymmetricGeometricDistanceTerms<double>(H,
		x1,
		x2,
		forward_error.data(),
		backward_error.data());
	return forward_error.squaredNorm() +
		backward_error.squaredNorm();
}

double CameraCalibrate_Ceres::calculate_median(std::vector<double> Data)
{
	double median; 
	size_t n = Data.size() / 2;
	std::nth_element(Data.begin(), Data.begin() + n, Data.end());
	return Data[n];
}

void CameraCalibrate_Ceres::calculate_cameras_RT(const std::vector<Eigen::Quaterniond> R_Left, const std::vector<Eigen::Quaterniond> R_Right,
	std::vector<Eigen::Vector3d> T_left, std::vector<Eigen::Vector3d> T_right,
	Eigen::Quaterniond& R_now, Eigen::Vector3d& T_now)
{
	std::vector<double> media_x, media_y, media_z, media_w, media_T1, media_T2, media_T3;
	for (int ii = 0; ii < R_Left.size(); ii++)
	{
		Eigen::Quaterniond R_temp = Eigen::Quaterniond(R_Right[ii].toRotationMatrix() * (R_Left[ii].toRotationMatrix().transpose()));
		Eigen::Vector3d T_temp = T_right[ii] - R_temp * T_left[ii];
		media_x.push_back(R_temp.x());
		media_y.push_back(R_temp.y());
		media_z.push_back(R_temp.z());
		media_w.push_back(R_temp.w());
		media_T1.push_back(T_temp.x());
		media_T2.push_back(T_temp.y());
		media_T3.push_back(T_temp.z());
	}
	R_now.x() = calculate_median(media_x);
	R_now.y() = calculate_median(media_y);
	R_now.z() = calculate_median(media_z);
	R_now.w() = calculate_median(media_w);
	T_now.x() = calculate_median(media_T1);
	T_now.y() = calculate_median(media_T2);
	T_now.z() = calculate_median(media_T3);
}

void CameraCalibrate_Ceres::calculate_camera_parame(int image_width, int image_height, std::vector<std::vector<cv::Point3f>> obj_pt, std::vector<std::vector<cv::Point2f>> img_pt,
	std::vector<std::vector<cv::Point2f>> board_pt, std::vector <double>& reproj_err, std::vector<Eigen::Quaterniond>& camQvec, std::vector<Eigen::Vector3d>& camTvec
	, std::vector<std::vector<cv::Point2f>>& Re_project_Map
	, double*& camK, double*& camDK, double*& camDP, double*& camDT
	, ceres::Solver::Summary *summary
	, unsigned int max_iter_num
	, double stop_value
	, unsigned int num_threads
	, unsigned int timeout
	, unsigned char Loss_type
	, double Loss_value
	, unsigned int Dis_K_num
	, unsigned int Dis_P_num
	, unsigned int Dis_T_num
	, bool Use_shear
	, bool Use_same_F)
{
	/*init guess*/
	std::vector<Eigen::Matrix3d> vec_h;
	calculate_homography(board_pt, img_pt, vec_h);
	Eigen::Matrix3d camera_matrix;
	std::vector<Eigen::MatrixXd> vec_extrinsics;
	calculate_camera_instrinsics(vec_h, image_width, image_height, camera_matrix);
	calculate_extrinsics(vec_h, camera_matrix, vec_extrinsics);

	if (!Use_same_F)
	{
		if (camK == nullptr || camK == NULL)
		{
			camK = new double[5];
		}
		camK[0] = camera_matrix(0);
		camK[1] = camera_matrix(4);
		camK[2] = camera_matrix(2);
		camK[3] = camera_matrix(5);
		camK[4] = camera_matrix(1);
		if (camDK == nullptr || camDK == NULL)
		{
			camDK = new double[6];
		}
		camDK[0] = 0; camDK[1] = 0; camDK[2] = 0;
		camDK[3] = 0; camDK[4] = 0; camDK[5] = 0;
		if (camDP == nullptr || camDP == NULL)
		{
			camDP = new double[2];
		}
		camDP[0] = 0; camDP[1] = 0;
		if (camDT == nullptr || camDT == NULL)
		{
			camDT = new double[4];
		}
		camDT[0] = 0; camDT[1] = 0; camDT[2] = 0; camDT[3] = 0;
		camQvec.clear();
		camTvec.clear();
		for (int ii = 0; ii < vec_extrinsics.size(); ii++)
		{
			Eigen::Matrix3d R_odom_curr_now = vec_extrinsics[ii].block<3, 3>(0, 0);
			Eigen::Quaterniond q_temp = Eigen::Quaterniond(R_odom_curr_now);
			q_temp.normalize();
			camQvec.push_back(q_temp);
			Eigen::Vector3d t_temp;
			t_temp.x() = vec_extrinsics[ii](0, 3);
			t_temp.y() = vec_extrinsics[ii](1, 3);
			t_temp.z() = vec_extrinsics[ii](2, 3);
			camTvec.push_back(t_temp);
		}

		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector3d obj(obj_pt[ii][jj].x, obj_pt[ii][jj].y, obj_pt[ii][jj].z);
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole::Create(obj, img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, camK, camK + 4,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
			}
			problem.SetParameterization(camQvec[ii].coeffs().data(), qvec_parameterization);
		}


		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			problem.SetParameterBlockConstant(camDK + tt);
		}

		switch (Dis_P_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDP);
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDT);
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		case 2:
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			problem.SetParameterBlockConstant(camK + 4);
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);
		Re_project_Map.clear();
		reproj_err.clear();
		double al_er = 0;
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			std::vector<cv::Point2f> err_temp;
			double sum_err = 0;
			double sum_num = 0;
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					Eigen::Quaternion<double> qvec(camQvec[ii]);
					Eigen::Matrix<double, 3, 1> tvec;
					tvec << camTvec[ii][0], camTvec[ii][1], camTvec[ii][2];

					Eigen::Vector3d obj(obj_pt[ii][jj].x, obj_pt[ii][jj].y, obj_pt[ii][jj].z);
					Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);

					Eigen::Matrix<double, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a)
						+ camDT[0] * r2 + camDT[2] * r4;
					double yd = b * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b)
						+ camDT[1] * r2 + camDT[3] * r4;

					double ud = camK[0] * xd + camK[4] * yd + camK[2];
					double vd = camK[1] * yd + camK[3];
					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				else
				{
					err_temp.push_back(cv::Point2f(-1, -1));
				}
			}
			Re_project_Map.push_back(err_temp);
			reproj_err.push_back(sum_err / sum_num);
			al_er += sum_err / sum_num;
		}
		reproj_err.push_back(al_er / (double(reproj_err.size())));
	}
	else
	{
		if (camK == nullptr || camK == NULL)
		{
			camK = new double[5];
		}
		camK[0] = (camera_matrix(0) + camera_matrix(4)) / 2.0;
		camK[1] = camera_matrix(6);
		camK[2] = camera_matrix(7);
		camK[3] = camera_matrix(1);
		if (camDK == nullptr || camDK == NULL)
		{
			camDK = new double[6];
		}
		camDK[0] = 0; camDK[1] = 0; camDK[2] = 0;
		camDK[3] = 0; camDK[4] = 0; camDK[5] = 0;
		if (camDP == nullptr || camDP == NULL)
		{
			camDP = new double[2];
		}
		camDP[0] = 0; camDP[1] = 0;
		if (camDT == nullptr || camDT == NULL)
		{
			camDT = new double[4];
		}
		camDT[0] = 0; camDT[1] = 0; camDT[2] = 0; camDT[3] = 0;
		camQvec.clear();
		camTvec.clear();
		for (int ii = 0; ii < vec_extrinsics.size(); ii++)
		{
			Eigen::Matrix3d R_odom_curr_now = vec_extrinsics[ii].block<3, 3>(0, 0);
			Eigen::Quaterniond q_temp = Eigen::Quaterniond(R_odom_curr_now);
			q_temp.normalize();
			camQvec.push_back(q_temp);
			Eigen::Vector3d t_temp;
			t_temp.x() = vec_extrinsics[ii](0, 3);
			t_temp.y() = vec_extrinsics[ii](1, 3);
			t_temp.z() = vec_extrinsics[ii](2, 3);
			camTvec.push_back(t_temp);
		}

		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					Eigen::Vector3d obj(obj_pt[ii][jj].x, obj_pt[ii][jj].y, obj_pt[ii][jj].z);
					Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
					ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_SameF::Create(obj, img);
					ceres::LossFunction* loss_function;
					switch (Loss_type)
					{
					case 0:
						loss_function = new ceres::TrivialLoss();
						break;
					case 1:
						loss_function = new ceres::HuberLoss(Loss_value);
						break;
					case 2:
						loss_function = new ceres::SoftLOneLoss(Loss_value);
						break;
					case 3:
						loss_function = new ceres::CauchyLoss(Loss_value);
						break;
					case 4:
						loss_function = new ceres::ArctanLoss(Loss_value);
						break;
					default:
						loss_function = new ceres::CauchyLoss(Loss_value);
						break;
					}
					problem.AddResidualBlock(cost_function, loss_function, camK, camK + 3,
						camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
				}
			}
			problem.SetParameterization(camQvec[ii].coeffs().data(), qvec_parameterization);
		}


		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			problem.SetParameterBlockConstant(camDK + tt);
		}

		switch (Dis_P_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDP);
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDT);
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		case 2:
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			problem.SetParameterBlockConstant(camK + 3);
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);

		camK[4] = camK[3];
		camK[3] = camK[2];
		camK[2] = camK[1];
		camK[1] = camK[0];
		Re_project_Map.clear();
		reproj_err.clear();
		double al_er = 0;
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			std::vector<cv::Point2f> err_temp;
			double sum_err = 0;
			double sum_num = 0;
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					Eigen::Quaternion<double> qvec(camQvec[ii]);
					Eigen::Matrix<double, 3, 1> tvec;
					tvec << camTvec[ii][0], camTvec[ii][1], camTvec[ii][2];

					Eigen::Vector3d obj(obj_pt[ii][jj].x, obj_pt[ii][jj].y, obj_pt[ii][jj].z);
					Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);

					Eigen::Matrix<double, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a)
						+ camDT[0] * r2 + camDT[2] * r4;
					double yd = b * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b)
						+ camDT[1] * r2 + camDT[3] * r4;

					double ud = camK[0] * xd + camK[4] * yd + camK[2];
					double vd = camK[1] * yd + camK[3];
					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				else
				{
					err_temp.push_back(cv::Point2f(-1, -1));
				}
			}
			Re_project_Map.push_back(err_temp);
			reproj_err.push_back(sum_err / sum_num);
			al_er += sum_err / sum_num;
		}
		reproj_err.push_back(al_er / (double(reproj_err.size())));
	}
}


void CameraCalibrate_Ceres::calculate_camera_parame_fixed_optimize(int points_num, int image_width, int image_height,  std::vector<int> fix_index, std::vector<int> zero_index
	, std::vector<std::vector<cv::Point2f>> img_pt,
	std::vector<std::vector<cv::Point2f>> board_pt, std::vector<cv::Point3f>& obj_pt, std::vector <double>& reproj_err,
	std::vector<Eigen::Quaterniond>& camQvec, std::vector<Eigen::Vector3d>& camTvec
	, std::vector<std::vector<cv::Point2f>>& Re_project_Map
	, double*& camK, double*& camDK, double*& camDP, double*& camDT
	, ceres::Solver::Summary *summary
	, unsigned int max_iter_num
	, double stop_value
	, unsigned int num_threads
	, unsigned int timeout
	, unsigned char Loss_type
	, double Loss_value
	, unsigned int Dis_K_num
	, unsigned int Dis_P_num
	, unsigned int Dis_T_num
	, bool Use_shear
	, bool Use_same_F)
{
	/*init guess*/
	std::vector<Eigen::Matrix3d> vec_h;
	calculate_homography(board_pt, img_pt, vec_h);
	Eigen::Matrix3d camera_matrix;
	std::vector<Eigen::MatrixXd> vec_extrinsics;
	calculate_camera_instrinsics(vec_h, image_width, image_height, camera_matrix);
	calculate_extrinsics(vec_h, camera_matrix, vec_extrinsics);
	double* opt_points = new double[3 * points_num];

	if (!Use_same_F)
	{
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			opt_points[ii * 3] = obj_pt[ii].x;
			opt_points[ii * 3 + 1] = obj_pt[ii].y;
			opt_points[ii * 3 + 2] = obj_pt[ii].z;
		}

		if (camK == nullptr || camK == NULL)
		{
			camK = new double[5];
		}
		camK[0] = camera_matrix(0);
		camK[1] = camera_matrix(4);
		camK[2] = camera_matrix(6);
		camK[3] = camera_matrix(7);
		camK[4] = camera_matrix(1);
		if (camDK == nullptr || camDK == NULL)
		{
			camDK = new double[6];
		}
		camDK[0] = 0; camDK[1] = 0; camDK[2] = 0;
		camDK[3] = 0; camDK[4] = 0; camDK[5] = 0;
		if (camDP == nullptr || camDP == NULL)
		{
			camDP = new double[2];
		}
		camDP[0] = 0; camDP[1] = 0;
		if (camDT == nullptr || camDT == NULL)
		{
			camDT = new double[4];
		}
		camDT[0] = 0; camDT[1] = 0; camDT[2] = 0; camDT[3] = 0;
		camQvec.clear();
		camTvec.clear();
		for (int ii = 0; ii < vec_extrinsics.size(); ii++)
		{
			Eigen::Matrix3d R_odom_curr_now = vec_extrinsics[ii].block<3, 3>(0, 0);
			Eigen::Quaterniond q_temp = Eigen::Quaterniond(R_odom_curr_now);
			q_temp.normalize();
			camQvec.push_back(q_temp);
			Eigen::Vector3d t_temp;
			t_temp.x() = vec_extrinsics[ii](0, 3);
			t_temp.y() = vec_extrinsics[ii](1, 3);
			t_temp.z() = vec_extrinsics[ii](2, 3);
			camTvec.push_back(t_temp);
		}

		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				bool if_fixed = false;
				bool if_zero = false;
				for (int pp = 0; pp < fix_index.size(); pp++)
				{
					if (jj == fix_index[pp])
					{
						if_fixed = true;
						break;
					}
				}
				for (int pp = 0; pp < zero_index.size(); pp++)
				{
					if (jj == zero_index[pp])
					{
						if_zero = true;
						break;
					}
				}
				if (if_fixed)
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector3d obj(obj_pt.at(jj).x, obj_pt.at(jj).y, obj_pt.at(jj).z);
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole::Create(obj, img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, camK, camK + 4,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
				else if (if_zero)
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_ZerosPointZ::Create(img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[jj * 3], camK, camK + 4,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
				else
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_unFixedPoint::Create(img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[jj * 3], camK, camK + 4,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
			}
			problem.SetParameterization(camQvec[ii].coeffs().data(), qvec_parameterization);
		}

		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			problem.SetParameterBlockConstant(camDK + tt);
		}

		switch (Dis_P_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDP);
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDT);
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		case 2:
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		default:
			break;
		}

		if(!Use_shear)
		{
			problem.SetParameterBlockConstant(camK + 4);
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);


		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			obj_pt[ii].x = opt_points[ii * 3];
			obj_pt[ii].y = opt_points[ii * 3 + 1];
			obj_pt[ii].z = opt_points[ii * 3 + 2];
		}

		Re_project_Map.clear();
		reproj_err.clear();
		double al_er = 0;
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			std::vector<cv::Point2f> err_temp;
			double sum_err = 0;
			double sum_num = 0;
			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					Eigen::Quaternion<double> qvec(camQvec[ii]);
					Eigen::Matrix<double, 3, 1> tvec;
					tvec << camTvec[ii][0], camTvec[ii][1], camTvec[ii][2];

					Eigen::Vector3d obj(obj_pt.at(jj).x, obj_pt.at(jj).y, obj_pt.at(jj).z);
					Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);

					Eigen::Matrix<double, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a)
						+ camDT[0] * r2 + camDT[2] * r4;
					double yd = b * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b)
						+ camDT[1] * r2 + camDT[3] * r4;

					double ud = camK[0] * xd + camK[4] * yd + camK[2];
					double vd = camK[1] * yd + camK[3];
					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				else
				{
					err_temp.push_back(cv::Point2f(-1, -1));
				}
			}
			Re_project_Map.push_back(err_temp);
			reproj_err.push_back(sum_err / sum_num);
			al_er += sum_err / sum_num;
		}
		reproj_err.push_back(al_er / (double(reproj_err.size())));
	}
	else
	{
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			opt_points[ii * 3] = obj_pt[ii].x;
			opt_points[ii * 3 + 1] = obj_pt[ii].y;
			opt_points[ii * 3 + 2] = obj_pt[ii].z;
		}

		if (camK == nullptr || camK == NULL)
		{
			camK = new double[5];
		}
		camK[0] = (camera_matrix(0) + camera_matrix(4)) / 2.0;
		camK[1] = camera_matrix(6);
		camK[2] = camera_matrix(7);
		camK[3] = camera_matrix(1);
		if (camDK == nullptr || camDK == NULL)
		{
			camDK = new double[6];
		}
		camDK[0] = 0; camDK[1] = 0; camDK[2] = 0;
		camDK[3] = 0; camDK[4] = 0; camDK[5] = 0;
		if (camDP == nullptr || camDP == NULL)
		{
			camDP = new double[2];
		}
		camDP[0] = 0; camDP[1] = 0;
		if (camDT == nullptr || camDT == NULL)
		{
			camDT = new double[4];
		}
		camDT[0] = 0; camDT[1] = 0; camDT[2] = 0; camDT[3] = 0;
		camQvec.clear();
		camTvec.clear();
		for (int ii = 0; ii < vec_extrinsics.size(); ii++)
		{
			Eigen::Matrix3d R_odom_curr_now = vec_extrinsics[ii].block<3, 3>(0, 0);
			Eigen::Quaterniond q_temp = Eigen::Quaterniond(R_odom_curr_now);
			q_temp.normalize();
			camQvec.push_back(q_temp);
			Eigen::Vector3d t_temp;
			t_temp.x() = vec_extrinsics[ii](0, 3);
			t_temp.y() = vec_extrinsics[ii](1, 3);
			t_temp.z() = vec_extrinsics[ii](2, 3);
			camTvec.push_back(t_temp);
		}

		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				bool if_fixed = false;
				bool if_zero = false;
				for (int pp = 0; pp < fix_index.size(); pp++)
				{
					if (jj == fix_index[pp])
					{
						if_fixed = true;
						break;
					}
				}
				for (int pp = 0; pp < zero_index.size(); pp++)
				{
					if (jj == zero_index[pp])
					{
						if_zero = true;
						break;
					}
				}
				if (if_fixed)
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector3d obj(obj_pt.at(jj).x, obj_pt.at(jj).y, obj_pt.at(jj).z);
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_SameF::Create(obj, img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, camK, camK + 3,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
				else if (if_zero)
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_ZerosPointZ_SameF::Create(img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[jj * 3], camK, camK + 3,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
				else
				{
					if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
					{
						Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_unFixedPoint_SameF::Create(img);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[jj * 3], camK, camK + 3,
							camDK, camDK + 1, camDK + 2, camDK + 3, camDK + 4, camDK + 5, camDP, camDT, camDT + 2, camQvec[ii].coeffs().data(), camTvec[ii].data());
					}
				}
			}
			problem.SetParameterization(camQvec[ii].coeffs().data(), qvec_parameterization);
		}

		for (int ss = Dis_K_num; ss < 6; ss++)
		{
			problem.SetParameterBlockConstant(camDK + ss);
		}

		switch (Dis_P_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDP);
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			problem.SetParameterBlockConstant(camDT);
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		case 2:
			problem.SetParameterBlockConstant(camDT + 2);
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			problem.SetParameterBlockConstant(camK + 3);
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);


		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			obj_pt[ii].x = opt_points[ii * 3];
			obj_pt[ii].y = opt_points[ii * 3 + 1];
			obj_pt[ii].z = opt_points[ii * 3 + 2];
		}

		camK[4] = camK[3];
		camK[3] = camK[2];
		camK[2] = camK[1];
		camK[1] = camK[0];

		Re_project_Map.clear();
		reproj_err.clear();
		double al_er = 0;
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			std::vector<cv::Point2f> err_temp;
			double sum_err = 0;
			double sum_num = 0;
			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				if (img_pt[ii][jj].x != 0 && img_pt[ii][jj].y != 0)
				{
					Eigen::Quaternion<double> qvec(camQvec[ii]);
					Eigen::Matrix<double, 3, 1> tvec;
					tvec << camTvec[ii][0], camTvec[ii][1], camTvec[ii][2];

					Eigen::Vector3d obj(obj_pt.at(jj).x, obj_pt.at(jj).y, obj_pt.at(jj).z);
					Eigen::Vector2d img(img_pt[ii][jj].x, img_pt[ii][jj].y);

					Eigen::Matrix<double, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a)
						+ camDT[0] * r2 + camDT[2] * r4;
					double yd = b * (1.0 + camDK[0] * r2 + camDK[1] * r4 + camDK[2] * r6)
						/ (1.0 + camDK[3] * r2 + camDK[4] * r4 + camDK[5] * r6)
						+ 2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b)
						+ camDT[1] * r2 + camDT[3] * r4;

					double ud = camK[0] * xd + camK[4] * yd + camK[2];
					double vd = camK[1] * yd + camK[3];
					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				else
				{
					err_temp.push_back(cv::Point2f(-1, -1));
				}
			}
			Re_project_Map.push_back(err_temp);
			reproj_err.push_back(sum_err / sum_num);
			al_er += sum_err / sum_num;
		}
		reproj_err.push_back(al_er / (double(reproj_err.size())));
	}
}


void CameraCalibrate_Ceres::calculate_camera_parame_Dual(std::vector<std::vector<std::vector<cv::Point3f>>> obj_pt,
	std::vector<std::vector<std::vector<cv::Point2f>>> img_pt, std::vector<std::vector <double>>& reproj_err,
	std::vector<std::vector<Eigen::Quaterniond>>& camQvec, std::vector<std::vector<Eigen::Vector3d>>& camTvec
	, std::vector<std::vector<std::vector<cv::Point2f>>>& Re_project_Map
	, std::vector<double*>& camK, std::vector<double*>& camDK, std::vector<double*>& camDP, std::vector<double*>& camDT,
	Eigen::Quaterniond& Dual_R, Eigen::Vector3d& Dual_T
	, ceres::Solver::Summary *summary
	, unsigned int max_iter_num
	, double stop_value
	, unsigned int num_threads
	, unsigned int timeout
	, unsigned char Loss_type
	, double Loss_value
	, unsigned int Dis_K_num
	, unsigned int Dis_P_num
	, unsigned int Dis_T_num
	, bool Use_shear
	, bool Use_same_F)
{
	/*RT init*/
	calculate_cameras_RT(camQvec[0], camQvec[1], camTvec[0], camTvec[1], Dual_R, Dual_T);
	if (!Use_same_F)
	{
		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < obj_pt[0].size(); ii++)
		{
			for (int kk = 0; kk < obj_pt[0][ii].size(); kk++)
			{
				Eigen::Vector3d obj(obj_pt[0][ii][kk].x, obj_pt[0][ii][kk].y, obj_pt[0][ii][kk].z);
				Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
				Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
				ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_Dual::Create(obj, img1, img2);
				ceres::LossFunction* loss_function;
				switch (Loss_type)
				{
				case 0:
					loss_function = new ceres::TrivialLoss();
					break;
				case 1:
					loss_function = new ceres::HuberLoss(Loss_value);
					break;
				case 2:
					loss_function = new ceres::SoftLOneLoss(Loss_value);
					break;
				case 3:
					loss_function = new ceres::CauchyLoss(Loss_value);
					break;
				case 4:
					loss_function = new ceres::ArctanLoss(Loss_value);
					break;
				default:
					loss_function = new ceres::CauchyLoss(Loss_value);
					break;
				}
				problem.AddResidualBlock(cost_function, loss_function
					, camK[0], camK[1], camK[0] + 4, camK[1] + 4
					, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
					, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
					, camDP[0], camDP[1]
					, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2
					, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
			}
			problem.SetParameterization(camQvec[0][ii].coeffs().data(), qvec_parameterization);
		}
		problem.SetParameterization(Dual_R.coeffs().data(), qvec_parameterization);

		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDK[ss] + tt);
			}
		}

		switch (Dis_P_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDP[ss]);
			}
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss]);
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		case 2:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camK[ss] + 4);
			}
		}
		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);
		Re_project_Map.clear();
		reproj_err.clear();
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			double al_er = 0;
			std::vector<std::vector<cv::Point2f>> err_temp_all;
			std::vector<double> reproj_err_temp_all;
			
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				std::vector<cv::Point2f> err_temp;
				double sum_err = 0;
				double sum_num = 0;
				for (int kk = 0; kk < obj_pt[ii][jj].size(); kk++)
				{

					Eigen::Vector3d obj(obj_pt[ii][jj][kk].x, obj_pt[ii][jj][kk].y, obj_pt[ii][jj][kk].z);
					Eigen::Vector2d img(img_pt[ii][jj][kk].x, img_pt[ii][jj][kk].y);
					Eigen::Matrix<double, 3, 1> obj_cam_coor;
					if (ii == 0)
					{
						Eigen::Quaternion<double> qvec(camQvec[0][jj]);
						Eigen::Matrix<double, 3, 1> tvec;
						tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
						obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;
					}
					else
					{
						Eigen::Quaternion<double> qvec(camQvec[0][jj]);
						Eigen::Matrix<double, 3, 1> tvec;
						tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
						obj_cam_coor = Dual_R.toRotationMatrix() * qvec.toRotationMatrix() * obj + Dual_T + Dual_R.toRotationMatrix() * tvec;
					}

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
						/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
						+ 2.0 * camDP[ii][0] * a * b + camDP[ii][1] * (r2 + 2.0 * a * a)
						+ camDT[ii][0] * r2 + camDT[ii][2] * r4;
					double yd = b * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
						/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
						+ 2.0 * camDP[ii][1] * a * b + camDP[ii][0] * (r2 + 2.0 * b * b)
						+ camDT[ii][1] * r2 + camDT[ii][3] * r4;

					double ud = camK[ii][0] * xd + camK[ii][4] * yd + camK[ii][2];
					double vd = camK[ii][1] * yd + camK[ii][3];

					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				err_temp_all.push_back(err_temp);
				reproj_err_temp_all.push_back(sum_err / sum_num);
				al_er += sum_err / sum_num;
			}
			reproj_err_temp_all.push_back(al_er / (double(reproj_err_temp_all.size())));
			Re_project_Map.push_back(err_temp_all);
			reproj_err.push_back(reproj_err_temp_all);
		}
	}
	else
	{
		for (int ss = 0; ss < 2; ss++)
		{
			camK[ss][0] = (camK[ss][0] + camK[ss][1]) / 2.0;
			camK[ss][1] = camK[ss][2];
			camK[ss][2] = camK[ss][3];
			camK[ss][3] = camK[ss][4];
		}
		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < obj_pt[0].size(); ii++)
		{
			for (int kk = 0; kk < obj_pt[0][ii].size(); kk++)
			{
				Eigen::Vector3d obj(obj_pt[0][ii][kk].x, obj_pt[0][ii][kk].y, obj_pt[0][ii][kk].z);
				Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
				Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
				ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_Dual_SameF::Create(obj, img1, img2);
				ceres::LossFunction* loss_function;
				switch (Loss_type)
				{
				case 0:
					loss_function = new ceres::TrivialLoss();
					break;
				case 1:
					loss_function = new ceres::HuberLoss(Loss_value);
					break;
				case 2:
					loss_function = new ceres::SoftLOneLoss(Loss_value);
					break;
				case 3:
					loss_function = new ceres::CauchyLoss(Loss_value);
					break;
				case 4:
					loss_function = new ceres::ArctanLoss(Loss_value);
					break;
				default:
					loss_function = new ceres::CauchyLoss(Loss_value);
					break;
				}
				problem.AddResidualBlock(cost_function, loss_function
					, camK[0], camK[1], camK[0] + 3, camK[1] + 3
					, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
					, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
					, camDP[0], camDP[1]
					, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2
					, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
			}
			problem.SetParameterization(camQvec[0][ii].coeffs().data(), qvec_parameterization);
		}
		problem.SetParameterization(Dual_R.coeffs().data(), qvec_parameterization);

		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDK[ss] + tt);
			}
		}

		switch (Dis_P_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDP[ss]);
			}
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss]);
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		case 2:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camK[ss] + 3);
			}
		}
		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);

		for (int ss = 0; ss < 2; ss++)
		{
			camK[ss][4] = camK[ss][3];
			camK[ss][3] = camK[ss][2];
			camK[ss][2] = camK[ss][1];
			camK[ss][1] = camK[ss][0];
		}

		Re_project_Map.clear();
		reproj_err.clear();
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			double al_er = 0;
			std::vector<std::vector<cv::Point2f>> err_temp_all;
			std::vector<double> reproj_err_temp_all;
			
			for (int jj = 0; jj < obj_pt[ii].size(); jj++)
			{
				std::vector<cv::Point2f> err_temp;
				double sum_err = 0;
				double sum_num = 0;
				for (int kk = 0; kk < obj_pt[ii][jj].size(); kk++)
				{

					Eigen::Vector3d obj(obj_pt[ii][jj][kk].x, obj_pt[ii][jj][kk].y, obj_pt[ii][jj][kk].z);
					Eigen::Vector2d img(img_pt[ii][jj][kk].x, img_pt[ii][jj][kk].y);
					Eigen::Matrix<double, 3, 1> obj_cam_coor;
					if (ii == 0)
					{
						Eigen::Quaternion<double> qvec(camQvec[0][jj]);
						Eigen::Matrix<double, 3, 1> tvec;
						tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
						obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;
					}
					else
					{
						Eigen::Quaternion<double> qvec(camQvec[0][jj]);
						Eigen::Matrix<double, 3, 1> tvec;
						tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
						obj_cam_coor = Dual_R.toRotationMatrix() * qvec.toRotationMatrix() * obj + Dual_T + Dual_R.toRotationMatrix() * tvec;
					}

					double a = obj_cam_coor(0) / obj_cam_coor(2);
					double b = obj_cam_coor(1) / obj_cam_coor(2);
					double r2 = (a * a + b * b);
					double r4 = r2 * r2;
					double r6 = r2 * r4;

					double xd = a * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
						/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
						+ 2.0 * camDP[ii][0] * a * b + camDP[ii][1] * (r2 + 2.0 * a * a)
						+ camDT[ii][0] * r2 + camDT[ii][2] * r4;
					double yd = b * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
						/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
						+ 2.0 * camDP[ii][1] * a * b + camDP[ii][0] * (r2 + 2.0 * b * b)
						+ camDT[ii][1] * r2 + camDT[ii][3] * r4;

					double ud = camK[ii][0] * xd + camK[ii][4] * yd + camK[ii][2];
					double vd = camK[ii][1] * yd + camK[ii][3];

					err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
					sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
					sum_num += 1;
				}
				err_temp_all.push_back(err_temp);
				reproj_err_temp_all.push_back(sum_err / sum_num);
				al_er += sum_err / sum_num;
			}
			reproj_err_temp_all.push_back(al_er / (double(reproj_err_temp_all.size())));
			Re_project_Map.push_back(err_temp_all);
			reproj_err.push_back(reproj_err_temp_all);
		}
	}
}

void CameraCalibrate_Ceres::calculate_camera_parame_fixed_optimize_Dual(int points_num, std::vector<int> fix_index, std::vector<int> zeros_index,
	std::vector<std::vector<std::vector<cv::Point2f>>> img_pt,
	std::vector<cv::Point3f>& obj_pt, std::vector<std::vector<double>>& reproj_err,
	std::vector<std::vector<Eigen::Quaterniond>>& camQvec, std::vector<std::vector<Eigen::Vector3d>>& camTvec
	, std::vector<std::vector<std::vector<cv::Point2f>>>& Re_project_Map
	, std::vector<double*>& camK, std::vector<double*>& camDK, std::vector<double*>& camDP, std::vector<double*>& camDT,
	Eigen::Quaterniond &Dual_R, Eigen::Vector3d &Dual_T
	, ceres::Solver::Summary *summary
	, unsigned int max_iter_num
	, double stop_value
	, unsigned int num_threads
	, unsigned int timeout
	, unsigned char Loss_type
	, double Loss_value
	, unsigned int Dis_K_num
	, unsigned int Dis_P_num
	, unsigned int Dis_T_num
	, bool Use_shear
	, bool Use_same_F)
{
	/*RT init*/
	calculate_cameras_RT(camQvec[0], camQvec[1], camTvec[0], camTvec[1], Dual_R, Dual_T);
	if (!Use_same_F)
	{
		double* opt_points = new double[3 * points_num];
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			opt_points[ii * 3] = obj_pt[ii].x;
			opt_points[ii * 3 + 1] = obj_pt[ii].y;
			opt_points[ii * 3 + 2] = obj_pt[ii].z;
		}
		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < img_pt[0].size(); ii++)
		{
			for (int kk = 0; kk < img_pt[0][ii].size(); kk++)
			{
				bool if_fixed = false;
				for (int pp = 0; pp < fix_index.size(); pp++)
				{
					if (kk == fix_index[pp])
					{
						if_fixed = true;
						break;
					}
				}
				bool if_zero = false;
				for (int pp = 0; pp < zeros_index.size(); pp++)
				{
					if (kk == zeros_index[pp])
					{
						if_zero = true;
						break;
					}
				}
				if (if_fixed)
				{
					Eigen::Vector3d obj(obj_pt[kk].x, obj_pt[kk].y, obj_pt[kk].z);

					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_Dual::Create(obj, img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function
							, camK[0], camK[1], camK[0] + 4, camK[1] + 4
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2
							, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
				else if (if_zero)
				{
					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual::Create(img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[kk * 3]
							, camK[0], camK[1], camK[0] + 4, camK[1] + 4
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
				else
				{
					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_unFixedPoint_Dual::Create(img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[kk * 3]
							, camK[0], camK[1], camK[0] + 4, camK[1] + 4
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
			}
			problem.SetParameterization(camQvec[0][ii].coeffs().data(), qvec_parameterization);
		}
		problem.SetParameterization(Dual_R.coeffs().data(), qvec_parameterization);


		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDK[ss] + tt);
			}
		}

		switch (Dis_P_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDP[ss]);
			}
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss]);
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		case 2:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camK[ss] + 4);
			}
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);


		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			obj_pt[ii].x = opt_points[ii * 3];
			obj_pt[ii].y = opt_points[ii * 3 + 1];
			obj_pt[ii].z = opt_points[ii * 3 + 2];
		}
		Re_project_Map.clear();
		reproj_err.clear();
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			double al_er = 0;
			std::vector<std::vector<cv::Point2f>> err_temp_all;
			std::vector<double> reproj_err_temp_all;

			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				std::vector<cv::Point2f> err_temp;
				double sum_err = 0;
				double sum_num = 0;
				for (int kk = 0; kk < img_pt[ii][jj].size(); kk++)
				{
					bool is_canbe = true;
					for (int tt = 0; tt < img_pt.size(); tt++)
					{
						if (img_pt[tt][jj][kk].x == 0 && img_pt[tt][jj][kk].y == 0)
						{
							is_canbe = false;
						}
					}
					if (!is_canbe)
					{
						Eigen::Vector3d obj(obj_pt[kk].x, obj_pt[kk].y, obj_pt[kk].z);
						Eigen::Vector2d img(img_pt[ii][jj][kk].x, img_pt[ii][jj][kk].y);
						Eigen::Matrix<double, 3, 1> obj_cam_coor;
						if (ii == 0)
						{
							Eigen::Quaternion<double> qvec(camQvec[0][jj]);
							Eigen::Matrix<double, 3, 1> tvec;
							tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
							obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;
						}
						else
						{
							Eigen::Quaternion<double> qvec(camQvec[0][jj]);
							Eigen::Matrix<double, 3, 1> tvec;
							tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
							obj_cam_coor = Dual_R.toRotationMatrix() * qvec.toRotationMatrix() * obj + Dual_T + Dual_R.toRotationMatrix() * tvec;
						}
						double a = obj_cam_coor(0) / obj_cam_coor(2);
						double b = obj_cam_coor(1) / obj_cam_coor(2);
						double r2 = (a * a + b * b);
						double r4 = r2 * r2;
						double r6 = r2 * r4;

						double xd = a * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
							/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
							+ 2.0 * camDP[ii][0] * a * b + camDP[ii][1] * (r2 + 2.0 * a * a)
							+ camDT[ii][0] * r2 + camDT[ii][2] * r4;
						double yd = b * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
							/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
							+ 2.0 * camDP[ii][1] * a * b + camDP[ii][0] * (r2 + 2.0 * b * b)
							+ camDT[ii][1] * r2 + camDT[ii][3] * r4;

						double ud = camK[ii][0] * xd + camK[ii][4] * yd + camK[ii][2];
						double vd = camK[ii][1] * yd + camK[ii][3];
						err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
						sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
						sum_num += 1;
					}
					else
					{
						err_temp.push_back(cv::Point2f(-1, -1));
					}
				}
				err_temp_all.push_back(err_temp);
				reproj_err_temp_all.push_back(sum_err / sum_num);
				al_er += sum_err / sum_num;
			}
			reproj_err_temp_all.push_back(al_er / (double(reproj_err_temp_all.size())));
			Re_project_Map.push_back(err_temp_all);
			reproj_err.push_back(reproj_err_temp_all);
		}
	}
	else
	{
		for (int ss = 0; ss < 2; ss++)
		{
			camK[ss][0] = (camK[ss][0] + camK[ss][1]) / 2.0;
			camK[ss][1] = camK[ss][2];
			camK[ss][2] = camK[ss][3];
			camK[ss][3] = camK[ss][4];
		}
		double* opt_points = new double[3 * points_num];
		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			opt_points[ii * 3] = obj_pt[ii].x;
			opt_points[ii * 3 + 1] = obj_pt[ii].y;
			opt_points[ii * 3 + 2] = obj_pt[ii].z;
		}
		ceres::Problem problem;
		ceres::LocalParameterization* qvec_parameterization = new ceres::EigenQuaternionParameterization;
		for (int ii = 0; ii < img_pt[0].size(); ii++)
		{
			for (int kk = 0; kk < img_pt[0][ii].size(); kk++)
			{
				bool if_fixed = false;
				for (int pp = 0; pp < fix_index.size(); pp++)
				{
					if (kk == fix_index[pp])
					{
						if_fixed = true;
						break;
					}
				}
				bool if_zero = false;
				for (int pp = 0; pp < zeros_index.size(); pp++)
				{
					if (kk == zeros_index[pp])
					{
						if_zero = true;
						break;
					}
				}
				if (if_fixed)
				{
					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector3d obj(obj_pt[kk].x, obj_pt[kk].y, obj_pt[kk].z);
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_Dual_SameF::Create(obj, img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function
							, camK[0], camK[1], camK[0] + 3, camK[1] + 3
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2
							, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
				else if (if_zero)
				{
					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual_SameF::Create(img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[kk * 3]
							, camK[0], camK[1], camK[0] + 3, camK[1] + 3
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
				else
				{
					if (img_pt[0][ii][kk].x != 0 && img_pt[0][ii][kk].y != 0 && img_pt[1][ii][kk].x != 0 && img_pt[1][ii][kk].y != 0)
					{
						Eigen::Vector2d img1(img_pt[0][ii][kk].x, img_pt[0][ii][kk].y);
						Eigen::Vector2d img2(img_pt[1][ii][kk].x, img_pt[1][ii][kk].y);
						ceres::CostFunction* cost_function = ProjectErrorCostFunctionPinehole_unFixedPoint_Dual_SameF::Create(img1, img2);
						ceres::LossFunction* loss_function;
						switch (Loss_type)
						{
						case 0:
							loss_function = new ceres::TrivialLoss();
							break;
						case 1:
							loss_function = new ceres::HuberLoss(Loss_value);
							break;
						case 2:
							loss_function = new ceres::SoftLOneLoss(Loss_value);
							break;
						case 3:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						case 4:
							loss_function = new ceres::ArctanLoss(Loss_value);
							break;
						default:
							loss_function = new ceres::CauchyLoss(Loss_value);
							break;
						}
						problem.AddResidualBlock(cost_function, loss_function, &opt_points[kk * 3]
							, camK[0], camK[1], camK[0] + 3, camK[1] + 3
							, camDK[0], camDK[0] + 1, camDK[0] + 2, camDK[0] + 3, camDK[0] + 4, camDK[0] + 5
							, camDK[1], camDK[1] + 1, camDK[1] + 2, camDK[1] + 3, camDK[1] + 4, camDK[1] + 5
							, camDP[0], camDP[1]
							, camDT[0], camDT[0] + 2, camDT[1], camDT[1] + 2, camQvec[0][ii].coeffs().data(), camTvec[0][ii].data(), Dual_R.coeffs().data(), Dual_T.data());
					}
				}
			}
			problem.SetParameterization(camQvec[0][ii].coeffs().data(), qvec_parameterization);
		}
		problem.SetParameterization(Dual_R.coeffs().data(), qvec_parameterization);


		for (int tt = Dis_K_num; tt < 6; tt++)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDK[ss] + tt);
			}
		}

		switch (Dis_P_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDP[ss]);
			}
			break;
		default:
			break;
		}

		switch (Dis_T_num)
		{
		case 0:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss]);
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		case 2:
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camDT[ss] + 2);
			}
			break;
		default:
			break;
		}

		if (!Use_shear)
		{
			for (int ss = 0; ss < 2; ss++)
			{
				problem.SetParameterBlockConstant(camK[ss] + 3);
			}
		}

		ceres::Solver::Options options;
		options.minimizer_progress_to_stdout = false;
		options.linear_solver_type = ceres::DENSE_SCHUR;
		options.max_num_iterations = max_iter_num;
		options.function_tolerance = stop_value;
		options.gradient_tolerance = stop_value;
		options.parameter_tolerance = stop_value;

		ceres::Solve(options, &problem, summary);

		for (int ss = 0; ss < 2; ss++)
		{
			camK[ss][4] = camK[ss][3];
			camK[ss][3] = camK[ss][2];
			camK[ss][2] = camK[ss][1];
			camK[ss][1] = camK[ss][0];
		}

		for (int ii = 0; ii < obj_pt.size(); ii++)
		{
			obj_pt[ii].x = opt_points[ii * 3];
			obj_pt[ii].y = opt_points[ii * 3 + 1];
			obj_pt[ii].z = opt_points[ii * 3 + 2];
		}
		Re_project_Map.clear();
		reproj_err.clear();
		for (int ii = 0; ii < img_pt.size(); ii++)
		{
			double al_er = 0;
			std::vector<std::vector<cv::Point2f>> err_temp_all;
			std::vector<double> reproj_err_temp_all;

			for (int jj = 0; jj < img_pt[ii].size(); jj++)
			{
				std::vector<cv::Point2f> err_temp;
				double sum_err = 0;
				double sum_num = 0;
				for (int kk = 0; kk < img_pt[ii][jj].size(); kk++)
				{
					bool is_canbe = true;
					for (int tt = 0; tt < img_pt.size(); tt++)
					{
						if (img_pt[tt][jj][kk].x == 0 && img_pt[tt][jj][kk].y == 0)
						{
							is_canbe = false;
						}
					}
					if (!is_canbe)
					{
						Eigen::Vector3d obj(obj_pt[kk].x, obj_pt[kk].y, obj_pt[kk].z);
						Eigen::Vector2d img(img_pt[ii][jj][kk].x, img_pt[ii][jj][kk].y);
						Eigen::Matrix<double, 3, 1> obj_cam_coor;
						if (ii == 0)
						{
							Eigen::Quaternion<double> qvec(camQvec[0][jj]);
							Eigen::Matrix<double, 3, 1> tvec;
							tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
							obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;
						}
						else
						{
							Eigen::Quaternion<double> qvec(camQvec[0][jj]);
							Eigen::Matrix<double, 3, 1> tvec;
							tvec << camTvec[0][jj][0], camTvec[0][jj][1], camTvec[0][jj][2];
							obj_cam_coor = Dual_R.toRotationMatrix() * qvec.toRotationMatrix() * obj + Dual_T + Dual_R.toRotationMatrix() * tvec;
						}
						double a = obj_cam_coor(0) / obj_cam_coor(2);
						double b = obj_cam_coor(1) / obj_cam_coor(2);
						double r2 = (a * a + b * b);
						double r4 = r2 * r2;
						double r6 = r2 * r4;

						double xd = a * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
							/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
							+ 2.0 * camDP[ii][0] * a * b + camDP[ii][1] * (r2 + 2.0 * a * a)
							+ camDT[ii][0] * r2 + camDT[ii][2] * r4;
						double yd = b * (1.0 + camDK[ii][0] * r2 + camDK[ii][1] * r4 + camDK[ii][2] * r6)
							/ (1.0 + camDK[ii][3] * r2 + camDK[ii][4] * r4 + camDK[ii][5] * r6)
							+ 2.0 * camDP[ii][1] * a * b + camDP[ii][0] * (r2 + 2.0 * b * b)
							+ camDT[ii][1] * r2 + camDT[ii][3] * r4;

						double ud = camK[ii][0] * xd + camK[ii][4] * yd + camK[ii][2];
						double vd = camK[ii][1] * yd + camK[ii][3];
						err_temp.push_back(cv::Point2f(ud - img(0), vd - img(1)));
						sum_err += sqrt((ud - img(0)) * (ud - img(0)) + (vd - img(1)) * (vd - img(1)));
						sum_num += 1;
					}
					else
					{
						err_temp.push_back(cv::Point2f(-1, -1));
					}
				}
				err_temp_all.push_back(err_temp);
				reproj_err_temp_all.push_back(sum_err / sum_num);
				al_er += sum_err / sum_num;
			}
			reproj_err_temp_all.push_back(al_er / (double(reproj_err_temp_all.size())));
			Re_project_Map.push_back(err_temp_all);
			reproj_err.push_back(reproj_err_temp_all);
		}
	}

}