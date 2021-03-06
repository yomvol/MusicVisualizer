This music visualizer has been made in order to practice in creating C++ desktop apps. It utilizes Cinder framework. A musical track is loaded into RAM, its next 20 milliseconds get analyzed with Fast Fourier Transform every cycle. All of the animations are dependent on a signal`s amplitude and frequency spectrum at a given moment. One of the visualization is based on Marching Squares algorithm, which is "embarrassingly parallel". Unfortunately, I could not manage to apply multithreading (compatible with Cinder) to this algorithm. The visualization could really benefit from performance improvement. Nevertheless, I hope that you are going to have fun using this app. Future updates will, probably, add multithreading and new, more gaze-captivating visualizations.

Upon opening choose a track. Supported file types are the following: .wav, .mp3, .mp4, .m4a, .flac.

NOTICE: All demonstrative gifs are recorded in 20 fps and may not depict actual user experience.

![1](https://user-images.githubusercontent.com/83629932/171857696-3859aad0-8a2e-4e8c-8bbd-36260a24422a.gif)
![2](https://user-images.githubusercontent.com/83629932/171857317-de2f649f-e7b2-45cc-9643-bb8c4284ffeb.gif)
![3](https://user-images.githubusercontent.com/83629932/171857358-223a7473-5c6e-4b7e-b9ed-e9c0bb5f174d.gif)

Copyright (c) 2014, The Cinder Project

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
