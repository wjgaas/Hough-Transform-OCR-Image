//
// Created by wen on 16/6/6.
//

#include "HoughDetectEdge.h"
#include "Tools.h"
#include <iostream>

HoughDetectEdge::HoughDetectEdge() { }

HoughDetectEdge::~HoughDetectEdge() { }

bool detect(cv::Mat &image, cv::Mat &dst) {
    return false;
}

void HoughDetectEdge::detect(cv::Mat &image, std::vector<cv::Vec4i> &edges) {
    edges.clear();
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, CV_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    canndyImage(gray);
    std::vector<std::vector<cv::Vec4i>> lines;
    houghLines(gray, lines);
    if (lines.size() != 4) return;
    mergeLines(lines, edges);
}

void HoughDetectEdge::canndyImage(cv::Mat &gray) {
    cv::Mat dst;
    cv::GaussianBlur(gray, dst, cv::Size(3, 3), 0, 0);
    cv::Laplacian(dst, dst, CV_16S);
    dst.convertTo(dst, CV_8U);
    cv::add(gray, dst, gray);
    cv::Canny(gray, gray, 50, 200, 3);

    cv::Mat grad_x, grad_y;
    cv::Sobel(gray, grad_x, CV_16S, 1, 0, 3);
    grad_x = cv::abs(grad_x);
    grad_x.convertTo(grad_x, CV_8U);
    cv::Sobel(gray, grad_y, CV_16S, 0, 1, 3);
    grad_y = cv::abs(grad_y);
    grad_y.convertTo(grad_y, CV_8U);
    dst = grad_x + grad_y;

#ifdef _DEBUG
    cv::imshow("canndyimage", gray);
    cv::waitKey();
#endif //end _DEBUG
}

void HoughDetectEdge::houghLines(cv::Mat &gray, std::vector<std::vector<cv::Vec4i>> &lines) {
    double const THETA = 30.0 / 180.0;
    // lines[0] top
    // lines[1] bottom
    // lines[2] left
    // lines[3] right
    lines.clear();
    std::vector<cv::Vec4i> tmplines;
    HoughLinesP(gray, tmplines, 1, CV_PI / 180, 50, 20, 10);

    std::vector<cv::Vec4i> ups;
    std::vector<cv::Vec4i> downs;
    std::vector<cv::Vec4i> lefts;
    std::vector<cv::Vec4i> rights;
    for (size_t i = 0; i < tmplines.size(); ++i) {
        cv::Vec4i &line = tmplines[i];
        //cv::line(gray, cv::Point(line[0], line[1]), cv::Point(line[2], line[3]), cv::Scalar(255 * (i <= 1), 0, 255 * (i>1)), 1, CV_AA);
        int detaX = abs(line[0] - line[2]);
        int detaY = abs(line[1] - line[3]);

        if (detaX > detaY && atan(1.0 * detaY / detaX) < THETA) //the direction of horizon
        {
            if (std::max(line[1], line[3]) < gray.rows / 3) {
                ups.emplace_back(line);
                continue;
            }
            if (std::max(line[1], line[3]) > gray.rows * 2 / 3) {
                downs.emplace_back(line);
                continue;
            }
        }
        if (detaX < detaY && atan(1.0 * detaX / detaY) < THETA) {
            if (std::max(line[0], line[2]) < gray.cols / 3) {
                lefts.emplace_back(line);
                continue;
            }
            if (std::max(line[0], line[2]) > gray.cols * 2 / 3) {
                rights.emplace_back(line);
                continue;
            }
        }
    }
    lines.emplace_back(ups);
    lines.emplace_back(downs);
    lines.emplace_back(lefts);
    lines.emplace_back(rights);
    //return  lines.size() == 4;
#ifdef _DEBUG
    cv::Mat cdst;
    cv::cvtColor(gray, cdst, CV_GRAY2BGR);
    for (size_t i = 0; i < lines.size(); ++i)
    {
        for (size_t j = 0; j < lines[i].size(); ++j)
        {
            cv::Vec4i& l = lines[i][j];
            cv::line(cdst, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(255 * (i<=1), 0, 255*(i>1)), 1, CV_AA);
        }
    }
    cv::imshow("lines", cdst);
    cv::waitKey();
#endif //end _DEBUG
}

void HoughDetectEdge::mergeLines(std::vector<std::vector<cv::Vec4i>> &lines, std::vector<cv::Vec4i> &edges) {
    for (size_t i = 0; i < lines.size(); ++i) {
        cv::Vec4i edge;
        Tools::mergeLines(lines[i], edge, static_cast<int>(i >= 2));
        edges.emplace_back(edge);
    }
}