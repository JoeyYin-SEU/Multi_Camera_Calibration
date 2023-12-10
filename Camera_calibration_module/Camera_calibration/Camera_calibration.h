#pragma once
#include<ceres/ceres.h>
#include <ceres/problem.h>
#include "Eigen/Eigen"
#include<opencv2/opencv.hpp>
#include <vector>
class ProjectErrorCostFunctionPinehole_unFixedPoint
{
public:
	ProjectErrorCostFunctionPinehole_unFixedPoint(const Eigen::Vector2d img_pixel) :
		 m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4, 
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{
		// 上面的参数类型为数组

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], obj_3d[2];


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) 
			/ (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) 
			/ (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[2];
		T vd = camK[1] * yd + camK[3];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_unFixedPoint, 2, 3, 4, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>
			(new ProjectErrorCostFunctionPinehole_unFixedPoint(img_pixel)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel;
};

class ProjectErrorCostFunctionPinehole_ZerosPointZ
{
public:
	ProjectErrorCostFunctionPinehole_ZerosPointZ(const Eigen::Vector2d img_pixel) :
		m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4, 
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{
		// 上面的参数类型为数组

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], 0;


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) 
			/ (1.0 + camDK_4[0] * r2 + camDK_5[1] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) 
			/ (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[2];
		T vd = camK[1] * yd + camK[3];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_ZerosPointZ, 2, 2, 4, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>(new ProjectErrorCostFunctionPinehole_ZerosPointZ(img_pixel)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel;
};

class ProjectErrorCostFunctionPinehole
{
public:
	ProjectErrorCostFunctionPinehole(const Eigen::Vector3d obj_pt, const Eigen::Vector2d img_pixel) : m_obj_pt(obj_pt), m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4,
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 3, 1> obj = m_obj_pt.cast<T>();
		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[2];
		T vd = camK[1] * yd + camK[3];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector3d obj_pt, Eigen::Vector2d img_pixel)
		{
			return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole, 2, 4, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>(new ProjectErrorCostFunctionPinehole(obj_pt, img_pixel)));
		}
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector3d m_obj_pt;
	const Eigen::Vector2d m_img_pixel;
};

class ProjectErrorCostFunctionPinehole_unFixedPoint_SameF
{
public:
	ProjectErrorCostFunctionPinehole_unFixedPoint_SameF(const Eigen::Vector2d img_pixel) :
		m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4,
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{
		// 上面的参数类型为数组

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], obj_3d[2];


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[1];
		T vd = camK[0] * yd + camK[2];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_unFixedPoint_SameF, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>
			(new ProjectErrorCostFunctionPinehole_unFixedPoint_SameF(img_pixel)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel;
};

class ProjectErrorCostFunctionPinehole_ZerosPointZ_SameF
{
public:
	ProjectErrorCostFunctionPinehole_ZerosPointZ_SameF(const Eigen::Vector2d img_pixel) :
		m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4,
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{
		// 上面的参数类型为数组

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], 0;


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[1];
		T vd = camK[0] * yd + camK[2];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_ZerosPointZ_SameF, 2, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>
			(new ProjectErrorCostFunctionPinehole_ZerosPointZ_SameF(img_pixel)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel;
};

class ProjectErrorCostFunctionPinehole_SameF
{
public:
	ProjectErrorCostFunctionPinehole_SameF(const Eigen::Vector3d obj_pt, const Eigen::Vector2d img_pixel) : m_obj_pt(obj_pt), m_img_pixel(img_pixel) {}

	template <typename T>
	bool operator()(const T* const camK, const T* const camK_shear,
		const T* const camDK_1, const T* const camDK_2, const T* const camDK_3, const T* const camDK_4, const T* const camDK_5, const T* const camDK_6,
		const T* const camDP, const T* const camDT_2, const T* const camDT_4,
		const T* const camQvec, const T* const camTvec, T* residuals) const
	{


		Eigen::Quaternion<T> qvec(camQvec);
		Eigen::Matrix<T, 3, 1> tvec;
		tvec << camTvec[0], camTvec[1], camTvec[2];

		Eigen::Matrix<T, 3, 1> obj = m_obj_pt.cast<T>();
		Eigen::Matrix<T, 2, 1> img = m_img_pixel.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor = qvec.toRotationMatrix() * obj + tvec;

		T a = obj_cam_coor(0) / obj_cam_coor(2);
		T b = obj_cam_coor(1) / obj_cam_coor(2);

		T r2 = (a * a + b * b);
		T r4 = r2 * r2;
		T r6 = r2 * r4;

		T xd = a * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[0] * a * b + camDP[1] * (r2 + 2.0 * a * a) + camDT_2[0] * r2 + +camDT_4[0] * r4;
		T yd = b * (1.0 + camDK_1[0] * r2 + camDK_2[0] * r4 + camDK_3[0] * r6) / (1.0 + camDK_4[0] * r2 + camDK_5[0] * r4 + camDK_6[0] * r6) +
			2.0 * camDP[1] * a * b + camDP[0] * (r2 + 2.0 * b * b) + camDT_2[1] * r2 + +camDT_4[1] * r4;

		T ud = camK[0] * xd + camK_shear[0] * yd + camK[1];
		T vd = camK[0] * yd + camK[2];

		// 残差 二维
		residuals[0] = ud - img(0);
		residuals[1] = vd - img(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector3d obj_pt, Eigen::Vector2d img_pixel)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_SameF, 2, 3, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 4, 3>
			(new ProjectErrorCostFunctionPinehole_SameF(obj_pt, img_pixel)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector3d m_obj_pt;
	const Eigen::Vector2d m_img_pixel;
};


class ProjectErrorCostFunctionPinehole_unFixedPoint_Dual
{
public:
	ProjectErrorCostFunctionPinehole_unFixedPoint_Dual(const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], obj_3d[2];

		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1) 
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[2];
		T vd_1 = camK_1[1] * yd_1 + camK_1[3];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[2];
		T vd_2 = camK_2[1] * yd_2 + camK_2[3];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_unFixedPoint_Dual,
			4, 3, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_unFixedPoint_Dual(img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};
class ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual
{
public:
	ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual(const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}
	
	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], 0;

		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[2];
		T vd_1 = camK_1[1] * yd_1 + camK_1[3];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[2];
		T vd_2 = camK_2[1] * yd_2 + camK_2[3];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);


		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual
			, 4, 2, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual(img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};
class ProjectErrorCostFunctionPinehole_Dual
{
public:
	ProjectErrorCostFunctionPinehole_Dual(const Eigen::Vector3d obj_pt, const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_obj_pt(obj_pt), m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}

	template <typename T>
	bool operator()(const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj = m_obj_pt.cast<T>();
		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[2];
		T vd_1 = camK_1[1] * yd_1 + camK_1[3];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[2];
		T vd_2 = camK_2[1] * yd_2 + camK_2[3];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector3d obj_pt, Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_Dual
			, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_Dual(obj_pt, img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector3d m_obj_pt;
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};

class ProjectErrorCostFunctionPinehole_unFixedPoint_Dual_SameF
{
public:
	ProjectErrorCostFunctionPinehole_unFixedPoint_Dual_SameF(const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], obj_3d[2];

		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[1];
		T vd_1 = camK_1[0] * yd_1 + camK_1[2];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[1];
		T vd_2 = camK_2[0] * yd_2 + camK_2[2];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_unFixedPoint_Dual_SameF,
			4, 3, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_unFixedPoint_Dual_SameF(img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};
class ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual_SameF
{
public:
	ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual_SameF(const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}

	template <typename T>
	bool operator()(const T* obj_3d, const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj;
		obj << obj_3d[0], obj_3d[1], 0;

		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[1];
		T vd_1 = camK_1[0] * yd_1 + camK_1[2];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[1];
		T vd_2 = camK_2[0] * yd_2 + camK_2[2];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);


		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual_SameF
			, 4, 2, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_ZerosPointZ_Dual_SameF(img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};
class ProjectErrorCostFunctionPinehole_Dual_SameF
{
public:
	ProjectErrorCostFunctionPinehole_Dual_SameF(const Eigen::Vector3d obj_pt, const Eigen::Vector2d img_pixel_1, const Eigen::Vector2d img_pixel_2) :
		m_obj_pt(obj_pt), m_img_pixel_1(img_pixel_1), m_img_pixel_2(img_pixel_2) {}

	template <typename T>
	bool operator()(const T* const camK_1, const T* const camK_2,
		const T* const camK_shear_1, const T* const camK_shear_2,
		const T* const camDK_1_1, const T* const camDK_1_2, const T* const camDK_1_3, const T* const camDK_1_4, const T* const camDK_1_5, const T* const camDK_1_6,
		const T* const camDK_2_1, const T* const camDK_2_2, const T* const camDK_2_3, const T* const camDK_2_4, const T* const camDK_2_5, const T* const camDK_2_6,
		const T* const camDP_1, const T* const camDP_2,
		const T* const camDT_1_1, const T* const camDT_1_2,
		const T* const camDT_2_1, const T* const camDT_2_2,
		const T* const camQvec_1, const T* const camTvec_1,
		const T* const Qvec, const T* const Tvec,
		T* residuals) const
	{
		Eigen::Quaternion<T> qvec_1(camQvec_1);
		Eigen::Matrix<T, 3, 1> tvec_1;
		tvec_1 << camTvec_1[0], camTvec_1[1], camTvec_1[2];

		Eigen::Quaternion<T> qvec_2(Qvec);
		Eigen::Matrix<T, 3, 1> tvec_2;
		tvec_2 << Tvec[0], Tvec[1], Tvec[2];

		Eigen::Matrix<T, 3, 1> obj = m_obj_pt.cast<T>();
		Eigen::Matrix<T, 2, 1> img_1 = m_img_pixel_1.cast<T>();
		Eigen::Matrix<T, 2, 1> img_2 = m_img_pixel_2.cast<T>();

		Eigen::Matrix<T, 3, 1> obj_cam_coor_1 = qvec_1.toRotationMatrix() * obj + tvec_1;
		T a_1 = obj_cam_coor_1(0) / obj_cam_coor_1(2);
		T b_1 = obj_cam_coor_1(1) / obj_cam_coor_1(2);
		T r2_1 = (a_1 * a_1 + b_1 * b_1);
		T r4_1 = r2_1 * r2_1;
		T r6_1 = r2_1 * r4_1;
		T xd_1 = a_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[0] * a_1 * b_1 + camDP_1[1] * (r2_1 + 2.0 * a_1 * a_1)
			+ camDT_1_1[0] * r2_1 + +camDT_1_2[0] * r4_1;
		T yd_1 = b_1 * (1.0 + camDK_1_1[0] * r2_1 + camDK_1_2[0] * r4_1 + camDK_1_3[0] * r6_1)
			/ (1.0 + camDK_1_4[0] * r2_1 + camDK_1_5[0] * r4_1 + camDK_1_6[0] * r6_1)
			+ 2.0 * camDP_1[1] * a_1 * b_1 + camDP_1[0] * (r2_1 + 2.0 * b_1 * b_1)
			+ camDT_1_1[1] * r2_1 + camDT_1_2[1] * r4_1;
		T ud_1 = camK_1[0] * xd_1 + camK_shear_1[0] * yd_1 + camK_1[1];
		T vd_1 = camK_1[0] * yd_1 + camK_1[2];

		Eigen::Matrix<T, 3, 1> obj_cam_coor_2 = qvec_2.toRotationMatrix() * qvec_1.toRotationMatrix() * obj + tvec_2 + qvec_2.toRotationMatrix() * tvec_1;
		T a_2 = obj_cam_coor_2(0) / obj_cam_coor_2(2);
		T b_2 = obj_cam_coor_2(1) / obj_cam_coor_2(2);
		T r2_2 = (a_2 * a_2 + b_2 * b_2);
		T r4_2 = r2_2 * r2_2;
		T r6_2 = r2_2 * r4_2;
		T xd_2 = a_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[0] * a_2 * b_2 + camDP_2[1] * (r2_2 + 2.0 * a_2 * a_2)
			+ camDT_2_1[0] * r2_2 + +camDT_2_2[0] * r4_2;
		T yd_2 = b_2 * (1.0 + camDK_2_1[0] * r2_2 + camDK_2_2[0] * r4_2 + camDK_2_3[0] * r6_2)
			/ (1.0 + camDK_2_4[0] * r2_2 + camDK_2_5[0] * r4_2 + camDK_2_6[0] * r6_2)
			+ 2.0 * camDP_2[1] * a_2 * b_2 + camDP_2[0] * (r2_2 + 2.0 * b_2 * b_2)
			+ camDT_2_1[1] * r2_2 + +camDT_2_2[1] * r4_2;
		T ud_2 = camK_2[0] * xd_2 + camK_shear_2[0] * yd_2 + camK_2[1];
		T vd_2 = camK_2[0] * yd_2 + camK_2[2];

		residuals[0] = ud_1 - img_1(0);
		residuals[1] = vd_1 - img_1(1);
		residuals[2] = ud_2 - img_2(0);
		residuals[3] = vd_2 - img_2(1);

		return true;
	}

	// 生成误差函数
	static ceres::CostFunction* Create(Eigen::Vector3d obj_pt, Eigen::Vector2d img_pixel_1, Eigen::Vector2d img_pixel_2)
	{
		return (new ceres::AutoDiffCostFunction<ProjectErrorCostFunctionPinehole_Dual_SameF
			, 4, 3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 4, 3, 4, 3>
			(new ProjectErrorCostFunctionPinehole_Dual_SameF(obj_pt, img_pixel_1, img_pixel_2)));
	}
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
	const Eigen::Vector3d m_obj_pt;
	const Eigen::Vector2d m_img_pixel_1;
	const Eigen::Vector2d m_img_pixel_2;
};

template <typename T>
void SymmetricGeometricDistanceTerms(const Eigen::Matrix<T, 3, 3>& H,
	const Eigen::Matrix<T, 2, 1>& x1,
	const Eigen::Matrix<T, 2, 1>& x2,
	T forward_error[2],
	T backward_error[2])
{
	typedef Eigen::Matrix<T, 3, 1> Vec3;
	Vec3 x(x1(0), x1(1), T(1.0));
	Vec3 y(x2(0), x2(1), T(1.0));

	Vec3 H_x = H * x;
	Vec3 Hinv_y = H.inverse() * y;

	H_x /= H_x(2);
	Hinv_y /= Hinv_y(2);

	forward_error[0] = H_x(0) - y(0);
	forward_error[1] = H_x(1) - y(1);
	backward_error[0] = Hinv_y(0) - x(0);
	backward_error[1] = Hinv_y(1) - x(1);
}


struct EstimateHomographyOptions {
	// Default settings for homography estimation which should be suitable
	// for a wide range of use cases.
	EstimateHomographyOptions()
		: max_num_iterations(50),
		expected_average_symmetric_distance(1e-16) {}

	int max_num_iterations;
	double expected_average_symmetric_distance;
};



struct HomographySymmetricGeometricCostFunctor
{
	HomographySymmetricGeometricCostFunctor(const Eigen::Vector2d& x,
		const Eigen::Vector2d& y)
		: x_(x), y_(y) { }

	template<typename T>
	bool operator()(const T* homography_parameters, T* residuals) const {
		typedef Eigen::Matrix<T, 3, 3> Mat3;
		typedef Eigen::Matrix<T, 2, 1> Vec2;

		Mat3 H(homography_parameters);
		Vec2 x(T(x_(0)), T(x_(1)));
		Vec2 y(T(y_(0)), T(y_(1)));

		SymmetricGeometricDistanceTerms<T>(H,
			x,
			y,
			&residuals[0],
			&residuals[2]);
		return true;
	}

	const Eigen::Vector2d x_;
	const Eigen::Vector2d y_;
};


class CameraCalibrate_Ceres
{
public:
	static void calculate_camera_parame(int image_width, int image_height, std::vector<std::vector<cv::Point3f>> obj_pt, std::vector<std::vector<cv::Point2f>> img_pt,
		std::vector<std::vector<cv::Point2f>> board_pt, std::vector <double>& reproj_err, 
		std::vector<Eigen::Quaterniond>& camQvec, std::vector<Eigen::Vector3d>& camTvec
		, std::vector<std::vector<cv::Point2f>>& Re_project_Map
		, double* &camK, double* &camDK, double* &camDP, double* &camDT
		,ceres::Solver::Summary *summary
		, unsigned int max_iter_num = 1000
		, double stop_value = 1e-9
		, unsigned int num_threads = 1
		, unsigned int timeout = 1e8
		, unsigned char Loss_type = 3
		, double Loss_value = 1
		, unsigned int Dis_K_num = 3
		, unsigned int Dis_P_num = 2
		, unsigned int Dis_T_num = 0
		, bool Use_shear = false
		, bool Use_same_F = false);

	static void calculate_camera_parame_fixed_optimize(int image_width, int image_height, int points_num, std::vector<int> fix_index, std::vector<int> zero_index,
		std::vector<std::vector<cv::Point2f>> img_pt,
		std::vector<std::vector<cv::Point2f>> board_pt, std::vector<cv::Point3f>& obj_pt, 
		std::vector <double>& reproj_err, std::vector<Eigen::Quaterniond>& camQvec, std::vector<Eigen::Vector3d>& camTvec
		, std::vector<std::vector<cv::Point2f>>& Re_project_Map
		, double* &camK, double* &camDK, double* &camDP, double* &camDT
		, ceres::Solver::Summary *summary
		, unsigned int max_iter_num = 1000
		, double stop_value = 1e-9
		, unsigned int num_threads = 1
		, unsigned int timeout = 1e8
		, unsigned char Loss_type = 3
		, double Loss_value = 1
		, unsigned int Dis_K_num = 3
		, unsigned int Dis_P_num = 2
		, unsigned int Dis_T_num = 0
		, bool Use_shear = false
		, bool Use_same_F = false);

	static void calculate_camera_parame_Dual(std::vector<std::vector<std::vector<cv::Point3f>>> obj_pt,
		std::vector<std::vector<std::vector<cv::Point2f>>> img_pt, std::vector<std::vector <double>>& reproj_err,
		std::vector<std::vector<Eigen::Quaterniond>>& camQvec, std::vector<std::vector<Eigen::Vector3d>>& camTvec
		, std::vector<std::vector<std::vector<cv::Point2f>>>& Re_project_Map
		, std::vector<double*>& camK, std::vector<double*>& camDK, std::vector<double*>& camDP, std::vector<double*>& camDT,
		Eigen::Quaterniond& Dual_R, Eigen::Vector3d& Dual_T
		, ceres::Solver::Summary *summary
		, unsigned int max_iter_num = 1000
		, double stop_value = 1e-9
		, unsigned int num_threads = 1
		, unsigned int timeout = 1e8
		, unsigned char Loss_type = 3
		, double Loss_value = 1
		, unsigned int Dis_K_num = 3
		, unsigned int Dis_P_num = 2
		, unsigned int Dis_T_num = 0
		, bool Use_shear = false
		, bool Use_same_F = false);

	static void calculate_camera_parame_fixed_optimize_Dual(int points_num, std::vector<int> fix_index, std::vector<int> zero_index,
		std::vector<std::vector<std::vector<cv::Point2f>>> img_pt,
		std::vector<cv::Point3f>& obj_pt, std::vector<std::vector<double>>& reproj_err,
		std::vector<std::vector<Eigen::Quaterniond>>& camQvec, std::vector<std::vector<Eigen::Vector3d>>& camTvec
		, std::vector<std::vector<std::vector<cv::Point2f>>>& Re_project_Map
		, std::vector<double*>& camK, std::vector<double*>& camDK, std::vector<double*>& camDP, std::vector<double*>& camDT,
		Eigen::Quaterniond &Dual_R, Eigen::Vector3d &Dual_T
		, ceres::Solver::Summary *summary
		, unsigned int max_iter_num = 1000
		, double stop_value = 1e-9
		, unsigned int num_threads = 1
		, unsigned int timeout = 1e8
		, unsigned char Loss_type = 3
		, double Loss_value = 1
		, unsigned int Dis_K_num = 3
		, unsigned int Dis_P_num = 2
		, unsigned int Dis_T_num = 0
		, bool Use_shear = false
		, bool Use_same_F = false);

	static double SymmetricGeometricDistance(const Eigen::Matrix3d& H, const Eigen::Vector2d& x1, const Eigen::Vector2d& x2);
private:
	static double calculate_median(std::vector<double> hWScores);
	static void calculate_cameras_RT(const std::vector<Eigen::Quaterniond> R_Left, const std::vector<Eigen::Quaterniond> R_Right, 
		std::vector<Eigen::Vector3d> T_left, std::vector<Eigen::Vector3d> T_right,
		Eigen::Quaterniond &R_now, Eigen::Vector3d & T_now);
	static void create_v(const Eigen::Matrix3d& h, const int p, const int q, Eigen::RowVectorXd& row_v);
	static void calculate_camera_instrinsics(const std::vector<Eigen::Matrix3d>& vec_h, int image_width, int image_height, Eigen::Matrix3d& camera_matrix);
	static void calculate_extrinsics(const std::vector<Eigen::Matrix3d>& vec_h_, const Eigen::Matrix3d& camera_matrix_, std::vector<Eigen::MatrixXd>& vec_extrinsics_);
	static void calculate_normalization_matrix(const std::vector<cv::Point2f>& pts, Eigen::Matrix3d& matrix_trans);
	static void calculate_distortion(std::vector<std::vector<cv::Point2f>> board_pts, std::vector<std::vector<cv::Point2f>> img_pts, const Eigen::Matrix3d& camera_matrix_,
		const std::vector<Eigen::MatrixXd>& vec_extrinsics_, Eigen::VectorXd& k_);
	static void calculate_homography(std::vector<std::vector<cv::Point2f>> board_pt, std::vector<std::vector<cv::Point2f>> img_pt, std::vector<Eigen::Matrix3d>& vec_hom);
	static void estimate_H(const std::vector<cv::Point2f>& img_pts, const std::vector<cv::Point2f>& board_pts, Eigen::Matrix3d& matrix_H);
};


class TerminationCheckingCallback : public ceres::IterationCallback {
public:
	TerminationCheckingCallback(const Eigen::MatrixXd& x1, const Eigen::MatrixXd& x2,
		const EstimateHomographyOptions& options,
		Eigen::Matrix3d* H)
		: options_(options), x1_(x1), x2_(x2), H_(H) {}

	virtual ceres::CallbackReturnType operator()(
		const ceres::IterationSummary& summary) {
		// If the step wasn't successful, there's nothing to do.
		if (!summary.step_is_successful) {
			return ceres::SOLVER_CONTINUE;
		}

		// Calculate average of symmetric geometric distance.
		double average_distance = 0.0;
		for (int i = 0; i < x1_.cols(); i++) {
			average_distance += CameraCalibrate_Ceres::SymmetricGeometricDistance(*H_,
				x1_.col(i),
				x2_.col(i));
		}
		average_distance /= x1_.cols();

		if (average_distance <= options_.expected_average_symmetric_distance) {
			return ceres::SOLVER_TERMINATE_SUCCESSFULLY;
		}
		return ceres::SOLVER_CONTINUE;
	}

private:
	const EstimateHomographyOptions& options_;
	const Eigen::MatrixXd& x1_;
	const Eigen::MatrixXd& x2_;
	Eigen::Matrix3d* H_;
};