%TRAININV
% Data generation script for the inverse kinematics demo
% - Data generation through direct kinematics
% - Data generation through inverse kinematics
%
% Code is written in unvectorized form for understandability. Refer the
% demo for the vectorized version of the code.
%
%   J.-S. Roger Jang, 6-28-94.
%   Madan Bharadwaj, 6-20-05
%   Copyright 1994-2005 The MathWorks, Inc. 



%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% DIRECT KINEMATICS
%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Deducing x and y coordinates using direct kinematics formulae
% x = f(theta1, theta2)
% y = g(theta1, theta2)

l1 = .77; % length of first arm
l2 = .265; % length of second arm

da = .585;  % length of bottom
ab = .77;   % First arm
bc = .265;  % second arm
ck = .115;   % final joint to tip of Kinect


theta1 = 0.2460914:0.01:2.2549654; % some first angle
theta2 = 1.094321:0.01:pi; % some second angle

thetaA = 0.8866273:0.1:2.8955012;   % 50.8 - 165.9  degrees
thetaB = 1.094321:0.1:pi;           % 62.7 - 180    degrees
thetaC = 1.307252:0.1:5.5065138;     % 74.9 - 315.5  degrees

data1 = [];
data2 = [];

[THETA1,THETA2] = meshgrid(theta1,theta2); % generate a grid of theta1 and theta2 values

X = l1 * cos(THETA1) + l2 * cos(THETA1 + THETA2); % compute x coordinates
Y = l1 * sin(THETA1) + l2 * sin(THETA1 + THETA2); % compute y coordinates

data1 = [X(:) Y(:) THETA1(:)]; % create x-y-theta1 dataset
data2 = [X(:) Y(:) THETA2(:)]; % create x-y-theta2 dataset

for i=1:1:length(theta1)
    for j=1:1:length(theta2)

        x = l1 * cos(theta1(i)) + l2 * cos(theta1(i)+theta2(j)); % x coordinate
        y = l1 * sin(theta1(i)) + l2 * sin(theta1(i)+theta2(j)); % y coordinate
        
        data2 = [data1; x y theta1(i)];
        data2 = [data2; x y theta2(j)];
       
    end
end



plot(X(:),Y(:),'r.');
  axis equal;
  xlabel('X','fontsize',10)
  ylabel('Y','fontsize',10)
  title('X-Y coordinates generated for all theta1 and theta2 combinations using forward kinematics formula','fontsize',10)
