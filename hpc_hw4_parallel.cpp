//
//  main.cpp
//  hpc_hw3
//
//  Created by Kevin Mueller on 2/6/19.
//  Copyright © 2019 Kevin Mueller. All rights reserved.
//

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "Jpegfile.h"
#include <cmath>
#include <omp.h>
#include <algorithm>    // std::transform
#include <vector>       // std::vector
#include <functional>   // std::plus
#include <time.h>

struct Pixel{
    int r,g,b;
    Pixel(int,int,int);
    Pixel();
    
    // member functions
    double l2norm ();
    Pixel operator- (const Pixel&);
    Pixel operator+ (const Pixel&);
    Pixel operator/ (const int&);
    bool operator== (const Pixel&);
    void operator+=(const Pixel & param);
    
    void print();
    BYTE* toDatabuf();
};


Pixel::Pixel(){}


Pixel::Pixel(int r,int g, int b){
    this->r = r;
    this->g = g;
    this->b = b;
}


Pixel Pixel::operator-(const Pixel & param){
    Pixel temp;
    temp.r = this->r - param.r;
    temp.g = this->g - param.g;
    temp.b = this->b - param.b;
    return temp;
    
}


Pixel Pixel::operator/ (const int &param){
    Pixel temp;
    // the plus 1 helps for divide by 0
    temp.r = (int) this->r/(param+1); 
    temp.g = (int) this->g/(param+1);
    temp.b = (int) this->b/(param+1);
    return temp;
    
}


Pixel Pixel::operator+(const Pixel & param){
    Pixel temp;
    temp.r = this->r + param.r;
    temp.g = this->g + param.g;
    temp.b = this->b + param.b;
    return temp;
    
}

void Pixel::operator+=(const Pixel & param){
   
    r +=  param.r;
    g +=  param.g;
    b +=  param.b;
    
}


bool Pixel::operator==(const Pixel & param){
    if(r == param.r && g == param.g && b == param.b){
        return true;
    }else{
        return false;
    }
}


double Pixel::l2norm(){
    return sqrt(pow(r,2)+pow(g,2)+pow(b,2));
}


Pixel* image_to_pixels(BYTE* dataBuf,int width,int height){
    
    Pixel* pixels = new Pixel[width*height];
    
    UINT row,col;
    for (row=0;row<height;row++) {
        for (col=0;col<width;col++) {
            BYTE *pRed, *pGrn, *pBlu;
            pRed = dataBuf + row * width * 3 + col * 3;
            pGrn = dataBuf + row * width * 3 + col * 3 + 1;
            pBlu = dataBuf + row * width * 3 + col * 3 + 2;
            pixels[row*width+col].r = (int) *pRed;
            pixels[row*width+col].g = (int) *pGrn;
            pixels[row*width+col].b = (int) *pBlu;
            }
        }
    
    return pixels;
}


void Pixel::print(){
    printf("r: %d g: %d b: %d \n",r,g,b);
}




bool check_color(Pixel pixel,Pixel* generators,int j){
    // Checks to see if the color is already defined
    for (int i=0;i<j;i++){
        if(generators[i] == pixel){
            return true;
            }
    }
    return false;
    
}


Pixel* get_generators(Pixel *image,int k,int total_pixels){
    
    Pixel* generators = new Pixel[k];
    int i = 0;
    while(i<k){
        int r = rand() % total_pixels;
        Pixel seed_color = image[r];
        if(~check_color(seed_color,generators,i)){
            generators[i] = seed_color;
            i++;
        }
    }
    return generators;
    
    
}




int main()
{

    srand(time(0));

	//printf("hey");
	UINT height;
	UINT width;
	BYTE *dataBuf;

	//read the file to dataBuf with RGB format
	dataBuf = JpegFile::JpegFileToRGB("Earth_Eastern_Hemisphere.jpg", &width, &height);
    
    
    Pixel* image = image_to_pixels(dataBuf, (int) width, (int) height);
    
    int total_pixels = (int) width * (int) height;
    int k = 5;
    

    Pixel *generators = get_generators(image, k, total_pixels);
    int* group_indices = new int[total_pixels];
    
    
    Pixel* k_sums = new Pixel[k];
    int* k_counts = new int[k];

    int tid; 
    omp_set_num_threads(4);

    double start_time = omp_get_wtime();



    // // Define user defined reduction
    // #pragma omp declare reduction( + : Pixel : \
    //                    std::plus< Pixel >() )\
    //                    initializer(omp_priv=Pixel(0,0,0))


    for(int iteration =0;iteration<10;iteration++){
        
        
        for(int i=0;i<k;i++){
            k_counts[i] = 0;
            k_sums[i] = Pixel(0,0,0);
        }
        
        // step 1 (initialization)

        // step 2 (group the pixels)
        // Parallelize this
        int min_index = -1;
        double dist,dist0;
        dist0= 100000000000.0;
        int row,col;
        #pragma omp parallel for private (row,col,min_index,dist,dist0) shared(group_indices,image,generators) schedule(dynamic)
                for(row=0;row<height;row++){
                    for(col=0;col<width;col++){
                        dist0= 100000000000.0;
                        min_index = -1;
                        for(int i=0;i<k;i++){


                            dist = (image[row*width+col] - generators[i]).l2norm();
                            
                            if(dist<dist0){
                                min_index = i;
                                dist0 = dist;
                            }
                        }
                        group_indices[row*width+col] = min_index;

                        k_sums[min_index] += image[row*width+col];
                        k_counts[min_index]++;
                    
                    }
                }


        


        // get new generators
        for(int i=0;i<k;i++){
            //k_sums[i].print();
            //std::cout << "counts: " << k_counts[i] << "\n";
            generators[i] = k_sums[i]/k_counts[i];
            //generators[i].print();
        }
    }   
    
    // convert image to dataBuf
    Pixel temp_gen;
    int idx;
    
    for(int row=0;row<height;row++){
        for(int col=0;col<width;col++){
            idx = group_indices[row*width+col];
            temp_gen = generators[idx];
            dataBuf[row * width * 3 + col * 3] = (BYTE)temp_gen.r;
            dataBuf[row * width * 3 + col * 3+1] = (BYTE)temp_gen.g;
            dataBuf[row * width * 3 + col * 3+2] = (BYTE)temp_gen.b;
            
        }
    }

    double total_seconds_elapsed = omp_get_wtime() - start_time;


    printf("total_seconds_elapsed: %f\n",total_seconds_elapsed);
    
    
	//write the gray luminance to another jpg file
	JpegFile::RGBToJpegFile("testmono.jpg", dataBuf, width, height, true, 75);
	
	delete dataBuf;
	return 1;
}
