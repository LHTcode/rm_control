//
// Created by qiayuan on 8/13/20.
//

#include "ori_tool.h"
#include "math_utilities.h"
#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Eigenvalues>

void quatToRPY(const geometry_msgs::Quaternion &q,
               double &roll, double &pitch, double &yaw) {

  double as = std::min(-2. * (q.x * q.z - q.w * q.y), .99999);
  yaw =
      std::atan2(2 * (q.x * q.y + q.w * q.z),
                 square(q.w) + square(q.x) - square(q.y)
                     - square(q.z));
  pitch = std::asin(as);
  roll =
      std::atan2(2 * (q.y * q.z + q.w * q.x),
                 square(q.w) - square(q.x) - square(q.y)
                     + square(q.z));
}

tf::Quaternion getAverageQuaternion(const std::vector<tf::Quaternion> &quaternions,
                                    const std::vector<double> &weights) {

  Eigen::MatrixXd Q = Eigen::MatrixXd::Zero(4, quaternions.size());
  Eigen::Vector3d vec;
  for (size_t i = 0; i < quaternions.size(); ++i) {
    // Weigh the quaternions according to their associated weight
    tf::Quaternion quat = quaternions[i] * weights[i];
    // Append the weighted Quaternion to a matrix Q.
    Q(0, i) = quat.x();
    Q(1, i) = quat.y();
    Q(2, i) = quat.z();
    Q(3, i) = quat.w();
  }
  // Creat a solver for finding the eigenvectors and eigenvalues
  Eigen::EigenSolver<Eigen::MatrixXd> es(Q * Q.transpose());
  // Find index of maximum (real) Eigenvalue.
  auto eigenvalues = es.eigenvalues();
  size_t max_idx = 0;
  double max_value = eigenvalues[max_idx].real();
  for (size_t i = 1; i < 4; ++i) {
    double real = eigenvalues[i].real();
    if (real > max_value) {
      max_value = real;
      max_idx = i;
    }
  }
  // Get corresponding Eigenvector, normalize it and return it as the average quat
  auto eigenvector = es.eigenvectors().col(max_idx).normalized();
  tf::Quaternion mean_orientation(
      eigenvector[0].real(),
      eigenvector[1].real(),
      eigenvector[2].real(),
      eigenvector[3].real()
  );
  return mean_orientation;
}
