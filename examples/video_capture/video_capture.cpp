#include <flow/flow.hpp>
#include <opencv2/opencv.hpp>
#include <spdlog/spdlog.h>


cv::VideoCapture capture;

cv::Mat capture_frame()
{
  cv::Mat frame;
  capture >> frame;
  return frame;
}

cv::Mat rotate(cv::Mat&& frame)
{
  cv::Mat src = std::move(frame);
  constexpr double angle = -45;

  // get rotation matrix for rotating the image around its center in pixel coordinates
  cv::Point2f center(
    static_cast<float>((src.cols - 1) / 2.0), static_cast<float>((src.rows - 1) / 2.0));
  cv::Mat rot = cv::getRotationMatrix2D(center, angle, 1.0);
  // determine bounding rectangle, center not relevant
  cv::Rect2f bbox = cv::RotatedRect(cv::Point2f(), src.size(), angle).boundingRect2f();
  // adjust transformation matrix
  rot.at<double>(0, 2) += static_cast<double>(bbox.width) / 2.0 - src.cols / 2.0;
  rot.at<double>(1, 2) += static_cast<double>(bbox.height) / 2.0 - src.rows / 2.0;

  cv::Mat dst;
  cv::warpAffine(src, dst, rot, bbox.size());
  return dst;
}

cv::Mat write(cv::Mat&& img)
{
  cv::imwrite("rotated_im.png", img);
  return img;
}

void show(cv::Mat&& frame)
{
  cv::imshow("Window", frame);
  cv::waitKey(30);
}

int main()
{
  using namespace flow::literals;

  constexpr int webcam = 0;
  capture.open(webcam);

  auto net = flow::network(flow::chain(60_q_Hz) | capture_frame | rotate | write | show);

  //  net.cancel_after(10s);
  flow::spin(std::move(net));
}
