/*
 * CNN demo for MNIST dataset
 * Author: Kai Han (kaihana@163.com)
 * Details in https://github.com/iamhankai/mini-dnn-cpp
 * Copyright 2018 Kai Han
 */
#include <Eigen/Dense>
#include <algorithm>
#include <iostream>
#include <time.h>

#include "src/layer.h"
#include "src/layer/conv.h"
#include "src/layer/fully_connected.h"
#include "src/layer/ave_pooling.h"
#include "src/layer/max_pooling.h"
#include "src/layer/relu.h"
#include "src/layer/sigmoid.h"
#include "src/layer/softmax.h"
#include "src/loss.h"
#include "src/loss/mse_loss.h"
#include "src/loss/cross_entropy_loss.h"
#include "src/mnist.h"
#include "src/network.h"
#include "src/optimizer.h"
#include "src/optimizer/sgd.h"


int main(int argc, char **argv) {
  // data (we're in "build" folder when run ./demo)
  MNIST dataset("../data/fashion_mnist/");
  dataset.read();
  int n_train = dataset.train_data.cols();
  int dim_in = dataset.train_data.rows();
  std::cout << "mnist train number: " << n_train << std::endl;
  std::cout << "mnist test number: " << dataset.test_labels.cols() << std::endl;
// std::cout << "dim_in (DIM IN): " << dim_in << std::endl;

  // dnn
  Network dnn;
  dnn.use_device = atoi(argv[1]);
  if(dnn.use_device > 0)
    std::cout << "Use device: filter type " << dnn.use_device << "\n";
  else if(dnn.use_device == 0)
    std::cout << "Use host\n";
  else
  {
    std::cout << "Cmd argument should be >= 0\n";
    exit(1);
  }
  
  Layer* C1 = new Conv(1, 28, 28, 6, 5, 5, 1, 0, 0);
  Layer* P2 = new MaxPooling(6, 24, 24, 2, 2, 2);
  Layer* C3 = new Conv(6, 12, 12, 16, 5, 5, 1, 0, 0);
  Layer* P4 = new MaxPooling(16, 8, 8, 2, 2, 2);
  Layer* fc6 = new FullyConnected(P4->output_dim(), 120);
  Layer* fc7 = new FullyConnected(120, 84);
  Layer* fc8 = new FullyConnected(84, 10);
  Layer* relu1 = new ReLU;
  Layer* relu3 = new ReLU;
  Layer* relu6 = new ReLU;
  Layer* relu7 = new ReLU;
  Layer* softmax = new Softmax;

  dnn.add_layer(C1);
  dnn.add_layer(relu1);
  dnn.add_layer(P2);
  dnn.add_layer(C3);
  dnn.add_layer(relu3);
  dnn.add_layer(P4);
  dnn.add_layer(fc6);
  dnn.add_layer(relu6);
  dnn.add_layer(fc7);
  dnn.add_layer(relu7);
  dnn.add_layer(fc8);
  dnn.add_layer(softmax);

  // Layer* conv1 = new Conv(1, 28, 28, 4, 5, 5, 2, 2, 2);
  // Layer* pool1 = new MaxPooling(4, 14, 14, 2, 2, 2);
  // Layer* conv2 = new Conv(4, 7, 7, 16, 5, 5, 1, 2, 2);
  // Layer* pool2 = new MaxPooling(16, 7, 7, 2, 2, 2);
  // Layer* fc3 = new FullyConnected(pool2->output_dim(), 32);
  // Layer* fc4 = new FullyConnected(32, 10);
  // Layer* relu1 = new ReLU;
  // Layer* relu2 = new ReLU;
  // Layer* relu3 = new ReLU;
  // Layer* softmax = new Softmax;
  // dnn.add_layer(conv1);
  // dnn.add_layer(relu1);
  // dnn.add_layer(pool1);
  // dnn.add_layer(conv2);
  // dnn.add_layer(relu2);
  // dnn.add_layer(pool2);
  // dnn.add_layer(fc3);
  // dnn.add_layer(relu3);
  // dnn.add_layer(fc4);
  // dnn.add_layer(softmax);

  // loss
  Loss* loss = new CrossEntropy;
  dnn.add_loss(loss);
  // train & test
  SGD opt(0.001, 5e-4, 0.9, true);
  // SGD opt(0.001);
  const int n_epoch = 5;
  const int batch_size = 128;

  time_t start, end;
  time(&start);

  for (int epoch = 0; epoch < n_epoch; epoch ++) {
    shuffle_data(dataset.train_data, dataset.train_labels);
    for (int start_idx = 0; start_idx < n_train; start_idx += batch_size) {
      int ith_batch = start_idx / batch_size;
      Matrix x_batch = dataset.train_data.block(0, start_idx, dim_in,
                                    std::min(batch_size, n_train - start_idx));
      Matrix label_batch = dataset.train_labels.block(0, start_idx, 1,
                                    std::min(batch_size, n_train - start_idx));
      Matrix target_batch = one_hot_encode(label_batch, 10);
      if (false && ith_batch % 10 == 1) {
        std::cout << ith_batch << "-th grad: " << std::endl;
        dnn.check_gradient(x_batch, target_batch, 10);
      }
      dnn.forward(x_batch);
      dnn.backward(x_batch, target_batch);
      // display
      if (ith_batch % 50 == 0) {
        std::cout << ith_batch << "-th batch, loss: " << dnn.get_loss()
        << std::endl;
      }
      // optimize
      dnn.update(opt);
    }
    // test
    dnn.forward(dataset.test_data);
    float acc = compute_accuracy(dnn.output(), dataset.test_labels);
    std::cout << std::endl;
    std::cout << epoch + 1 << "-th epoch, test acc: " << acc << std::endl;
    std::cout << std::endl;
  }

  time(&end);
  // Calculating total time taken by the program. 
  double time_taken = double(end - start); 
  std::cout << "Time taken by program is: " << time_taken << " seconds" << std::endl;

  return 0;
}

