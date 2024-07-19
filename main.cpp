#include <iostream>
#include <fstream>
#include <png.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <gtk/gtk.h>
#include <cstdlib>
#include <ctime>
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <queue>
#include <algorithm>
#include <limits>
using namespace std;

#define window_width 1600
#define window_height 900
#define fps 60

int w,h;

vector<vector<int>> inp(const char* s) {
    FILE *fp = fopen(s, "rb");
    if (!fp) {
        cerr << "Error: Could not open file" << endl;
        cout<<"Failure"<<endl;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        cerr << "Error: Could not allocate memory for png_ptr" << endl;
        fclose(fp);
        cout<<"Failure"<<endl;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        cerr << "Error: Could not allocate memory for info_ptr" << endl;
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        cout<<"Failure"<<endl;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        cerr << "Error: Could not set up error handling" << endl;
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        cout<<"Failure"<<endl;
    }

    png_init_io(png_ptr, fp);
    png_read_info(png_ptr, info_ptr);

    png_uint_32 width = png_get_image_width(png_ptr, info_ptr);
    png_uint_32 height = png_get_image_height(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);

    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
    }

    png_read_update_info(png_ptr, info_ptr);

    vector<png_bytep> row_pointers(height); // Use a vector of png_bytep pointers

    // Resize the vector to allocate memory for each row
    for (auto& row : row_pointers) {
        row = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
    }

    // Read the image data
    png_read_image(png_ptr, row_pointers.data()); // Pass the vector's data pointer

	 // Print the pixel data
	vector<vector<int>> im(height,vector<int>(width*4));
    for (png_uint_32 y = 0; y < height; y++) {
        for (png_uint_32 x = 0; x < width * 4; x++) {
            im[y][x]=static_cast<int>(row_pointers[y][x]);
        }
        cout << endl;
    }
    h=height;
    w=width;

    // Clean up
    for (auto& row : row_pointers){
        delete[] row; // Delete each row
    }
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);
    return im;
}


int op(vector<vector<int>> arr){

    vector<unsigned char> image(w * h * 4);

    int k=0;
    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w*4; j++) {
        	image[k] = arr[i][j];
        	k++;
        }
    }


    FILE *fp = fopen("output.png", "wb");
    if (!fp) {
        cerr << "Could not open file for writing" << endl;
        return 1;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        cerr << "Could not create PNG write struct" << endl;
        fclose(fp);
        return 1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        cerr << "Could not create PNG info struct" << endl;
        png_destroy_write_struct(&png_ptr, NULL);
        fclose(fp);
        return 1;
    }

    png_init_io(png_ptr, fp);

    png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    png_write_info(png_ptr, info_ptr);

    png_bytep row_ptr = new png_byte[w * 4];
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            row_ptr[x * 4] = image[(y * w + x) * 4];
            row_ptr[x * 4 + 1] = image[(y * w + x) * 4 + 1];
            row_ptr[x * 4 + 2] = image[(y * w + x) * 4 + 2];
            row_ptr[x * 4 + 3] = image[(y * w + x) * 4 + 3];
        }
        png_write_row(png_ptr, row_ptr);
    }

    delete[] row_ptr;

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    return 0;
}

class Util{
private:
	vector<vector<int>> applyBlur(vector<vector<int>>& image,int n) {
		if (n <= 10 && n >= 1) {
			int m = 2 * n + 1;
			float matrix[m][m];
			for (int i = 0; i < m; i++) {
				for (int j = 0; j < m; j++) {
					matrix[i][j] = 1.0 / (m * m);
				}
			}

			int height = image.size();
			int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
			int pad = m / 2;

			// Create a padded image with reflection
			vector<vector<int>> paddedImage(height + 2 * pad, vector<int>((width + 2 * pad) * 4, 0));

			// Copy the original image into the center of the padded image
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					for (int c = 0; c < 4; ++c) {
						paddedImage[y + pad][(x + pad) * 4 + c] = image[y][x * 4 + c];
					}
				}
			}

			// Reflect padding horizontally
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < pad; ++x) {
					for (int c = 0; c < 4; ++c) {
						paddedImage[y + pad][(pad - 1 - x) * 4 + c] = image[y][x * 4 + c];
						paddedImage[y + pad][(width + pad + x) * 4 + c] = image[y][(width - 1 - x) * 4 + c];
					}
				}
			}

			// Reflect padding vertically
			for (int x = 0; x < width + 2 * pad; ++x) {
				for (int y = 0; y < pad; ++y) {
					for (int c = 0; c < 4; ++c) {
						paddedImage[pad - 1 - y][x * 4 + c] = paddedImage[pad + y][x * 4 + c];
						paddedImage[height + pad + y][x * 4 + c] = paddedImage[height + pad - 1 - y][x * 4 + c];
					}
				}
			}

			// Create a temporary image to store blurred pixels
			vector<vector<int>> tempImage(height + 2 * pad, vector<int>((width + 2 * pad) * 4, 0));

			// Apply blur filter to each pixel in the padded image
			for (int y = pad; y < height + pad; ++y) {
				for (int x = pad; x < width + pad; ++x) {
					for (int c = 0; c < 4; ++c) { // Iterate over RGBA channels
						float sum = 0.0;
						for (int i = -pad; i <= pad; i++) {
							for (int j = -pad; j <= pad; j++) {
								sum += matrix[i + pad][j + pad] * paddedImage[y + i][4 * (x + j) + c];
							}
						}
						tempImage[y][4 * x + c] = static_cast<int>(sum);
					}
				}
			}

			// Copy the processed pixels back to the original image (excluding padding)
			for (int y = 0; y < height; ++y) {
				for (int x = 0; x < width; ++x) {
					for (int c = 0; c < 4; ++c) {
						image[y][4 * x + c] = tempImage[y + pad][4 * (x + pad) + c];
					}
				}
			}
		} else {
			cout << "Please enter a number between 1 and 10" << endl;
		}
		return image;
	}

	vector<vector<int>> sharpenImage(vector<vector<int>>& image, vector<vector<int>>& output) {
	    int h = image.size();
	    int w = image[0].size() / 4;

	    // Create a padded image with reflective padding
	    vector<vector<int>> paddedImage(h + 2, vector<int>(4 * (w + 2), 0));

	    // Copy the original image into the padded image
	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            for (int c = 0; c < 4; ++c) {
	                paddedImage[y + 1][4 * (x + 1) + c] = image[y][4 * x + c];
	            }
	        }
	    }

	    // Apply reflective padding to the top and bottom rows
	    for (int x = 0; x < w; ++x) {
	        for (int c = 0; c < 4; ++c) {
	            paddedImage[0][4 * (x + 1) + c] = image[1][4 * x + c];  // Top row
	            paddedImage[h + 1][4 * (x + 1) + c] = image[h - 2][4 * x + c];  // Bottom row
	        }
	    }

	    // Apply reflective padding to the left and right columns
	    for (int y = 0; y < h; ++y) {
	        for (int c = 0; c < 4; ++c) {
	            paddedImage[y + 1][4 * 0 + c] = image[y][4 * 1 + c];  // Left column
	            paddedImage[y + 1][4 * (w + 1) + c] = image[y][4 * (w - 2) + c];  // Right column
	        }
	    }

	    // Apply reflective padding to the corners
	    for (int c = 0; c < 4; ++c) {
	        paddedImage[0][4 * 0 + c] = image[1][4 * 1 + c];  // Top-left corner
	        paddedImage[0][4 * (w + 1) + c] = image[1][4 * (w - 2) + c];  // Top-right corner
	        paddedImage[h + 1][4 * 0 + c] = image[h - 2][4 * 1 + c];  // Bottom-left corner
	        paddedImage[h + 1][4 * (w + 1) + c] = image[h - 2][4 * (w - 2) + c];  // Bottom-right corner
	    }

	    // Apply sharpening to the padded image
	    for (int y = 1; y < h + 1; ++y) {
	        for (int x = 1; x < w + 1; ++x) {
	            for (int c = 0; c < 4; ++c) {
	                if (c == 3) {
	                    output[y - 1][4 * (x - 1) + c] = paddedImage[y][4 * x + c];
	                } else {
	                    int value = 5 * paddedImage[y][4 * x + c]
	                        - paddedImage[y - 1][4 * x + c]
	                        - paddedImage[y + 1][4 * x + c]
	                        - paddedImage[y][4 * (x - 1) + c]
	                        - paddedImage[y][4 * (x + 1) + c];
	                    output[y - 1][4 * (x - 1) + c] = min(max(value, 0), 255);
	                }
	            }
	        }
	    }
	    return paddedImage;
	}

	vector<vector<int>> enhanceImage(vector<vector<int>>& image, float contrast, int brightness) {
	    // Contrast adjustment
	    // New value = ((Old value - 128) * contrast) + 128
	    // Brightness adjustment
	    // New value = Old value + brightness
	    contrast=1+contrast/100;
	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            for (int c = 0; c < 3; ++c) { // Apply to RGB channels only
	                int newValue = (int)(((image[y][4 * x + c] - 128) * contrast) + 128 + brightness);//static_cast<int>
	                image[y][4 * x + c] = min(max(newValue, 0), 255);
	            }
	        }
	    }
	return image;
	}

	vector<vector<int>> scaleImage(vector<vector<int>>& image, float xScale, float yScale) {
	    int originalHeight = image.size();
	    int originalWidth = image[0].size() / 4; // Each pixel has 4 components (RGBA)
	    int newWidth = static_cast<int>(originalWidth * xScale);
	    int newHeight = static_cast<int>(originalHeight * yScale);
	    vector<vector<int>> scaledImage(newHeight, vector<int>(newWidth * 4, 0));

	    for (int y = 0; y < newHeight; ++y) {
	        for (int x = 0; x < newWidth; ++x) {
	            int srcX = static_cast<int>(x / xScale);
	            int srcY = static_cast<int>(y / yScale);
	            srcX = min(max(srcX, 0), originalWidth - 1); // Ensure srcX is within bounds
	            srcY = min(max(srcY, 0), originalHeight - 1); // Ensure srcY is within bounds
	            for (int c = 0; c < 4; ++c) {
	                scaledImage[y][4 * x + c] = image[srcY][4 * srcX + c];
	            }
	        }
	    }

	    return scaledImage;
	}

	vector<vector<int>> flipImageHor(vector<vector<int>>& image) {
	    int height = image.size();
	    int	 width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	    int direction;
	    vector<vector<int>> flippedImage = image;
	        for (int y = 0; y < height; ++y) {
	            for (int x = 0; x < width / 2; ++x) {
	                for (int c = 0; c < 4; ++c) {
	                    swap(flippedImage[y][4 * x + c], flippedImage[y][4 * (width - 1 - x) + c]);
	                }
	            }
	        }
	    return flippedImage;
	}

	vector<vector<int>> flipImageVert(vector<vector<int>>& image) {
	    int height = image.size();
	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	    vector<vector<int>> flippedImage = image;
	        for (int y = 0; y < height / 2; ++y) {
	            for (int x = 0; x < width; ++x) {
	                for (int c = 0; c < 4; ++c) {
	                    swap(flippedImage[y][4 * x + c], flippedImage[height - 1 - y][4 * x + c]);
	                }
	            }
	        }
	    return flippedImage;
	}

	vector<vector<int>> Canvas( vector<vector<int>>& image) {
	    // Sobel operator kernels
	    vector<vector<int>> sobelX = {
	        { -1, 0, 1 },
	        { -2, 0, 2 },
	        { -1, 0, 1 }
	    };

	    vector<vector<int>> sobelY = {
	        { -1, -2, -1 },
	        {  0,  0,  0 },
	        {  1,  2,  1 }
	    };

	    vector<vector<int>> edges(h, vector<int>(w * 4, 0)); // Initialize the edges image

	    // Detect edges using Sobel operator
	    for (int y = 1; y < h - 1; ++y) {
	        for (int x = 1; x < w - 1; ++x) {
	            int gx = 0;
	            int gy = 0;
	            for (int i = -1; i <= 1; ++i) {
	                for (int j = -1; j <= 1; ++j) {
	                    gx += image[y + i][4 * (x + j)] * sobelX[i + 1][j + 1];
	                    gy += image[y + i][4 * (x + j)] * sobelY[i + 1][j + 1];
	                }
	            }
	            int g = sqrt(gx * gx + gy * gy);
	            g = min(max(g, 0), 255);
	            if (g > 128) { // If it's an edge pixel, set to white (foreground)
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = 255;
	                }
	            } else { // Otherwise, set to black (background)
	                for (int c = 0; c < 3; c++) {
	                    edges[y][4 * x + c] = 0;
	                }
	            }
	        }
	    }
	        for (int y = 0; y < edges.size(); ++y) {
	        for (int x = 0; x < edges[0].size(); x += 4) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	        }
	    }
	    return edges;
	}

	vector<vector<int>> Saturate( vector<vector<int>>& image) {
	    int height = image.size();
	    int width = image[0].size();
	    vector<vector<int>> bwImage(height, vector<int>(width, 0));

	    for (int y = 0; y < height; ++y) {
	        for (int x = 0; x < width; ++x) {
	            int intensity = image[y][x];
	            bwImage[y][x] = (intensity > 128) ? 255 : 0; // Set pixel to white if intensity > 128, otherwise black
	        }
	    }

	    return bwImage;
	}

	vector<vector<int>> pixelateImage( vector<vector<int>>& image, int pixelationLevel) {
	    if (pixelationLevel < 1) pixelationLevel = 1;
	    int newWidth = w;
	    int newHeight = h;
	    vector<vector<int>> pixelatedImage(newHeight, vector<int>(newWidth * 4));
	    for (int y = 0; y < newHeight; y += pixelationLevel) {
	        for (int x = 0; x < newWidth; x += pixelationLevel) {
	            int rSum = 0, gSum = 0, bSum = 0, aSum = 0, count = 0;
	            for (int j = 0; j < pixelationLevel && (y + j) < newHeight; ++j) {
	                for (int i = 0; i < pixelationLevel && (x + i) < newWidth; ++i) {
	                    int idx = (y + j) * newWidth * 4 + (x + i) * 4;
	                    rSum += image[y + j][(x + i) * 4];
	                    gSum += image[y + j][(x + i) * 4 + 1];
	                    bSum += image[y + j][(x + i) * 4 + 2];
	                    aSum += image[y + j][(x + i) * 4 + 3];
	                    count++;
	                }
	            }
	            int rAvg = rSum / count;
	            int gAvg = gSum / count;
	            int bAvg = bSum / count;
	            int aAvg = aSum / count;
	            for (int j = 0; j < pixelationLevel && (y + j) < newHeight; ++j) {
	                for (int i = 0; i < pixelationLevel && (x + i) < newWidth; ++i) {
	                    pixelatedImage[y + j][(x + i) * 4] = rAvg;
	                    pixelatedImage[y + j][(x + i) * 4 + 1] = gAvg;
	                    pixelatedImage[y + j][(x + i) * 4 + 2] = bAvg;
	                    pixelatedImage[y + j][(x + i) * 4 + 3] = aAvg;
	                }
	            }
	        }
	    }
	    return pixelatedImage;
	}

	vector<vector<int>> convertToGrayscale( vector<vector<int>>& image) {
	    int height = image.size();
	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	    vector<vector<int>> grayImage(height, vector<int>(width * 4, 0));
	    for (int y = 0; y < height; ++y) {
	        for (int x = 0; x < width; ++x) {
	            int r = image[y][4 * x];
	            int g = image[y][4 * x + 1];
	            int b = image[y][4 * x + 2];
	            // Convert to grayscale using the formula
	            int gray = static_cast<int>(0.299 * r + 0.587 * g + 0.114 * b);
	            for (int c = 0; c < 3; ++c) { // Apply to R, G, B channels
	                grayImage[y][4 * x + c] = gray;
	            }
	            grayImage[y][4 * x + 3] = image[y][4 * x + 3]; // Preserve the alpha channel
	        }
	    }
	    return grayImage;
	}
	// Function to make black pixels transparent
	vector<vector<int>> makeBlackTransparent( vector<vector<int>>& image) {
	    int height = image.size();
	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	    vector<vector<int>> newImage = image;

	    for (int y = 0; y < height; ++y) {
	        for (int x = 0; x < width; ++x) {
	            bool isBlack = true;
	            for (int c = 0; c < 3; ++c) { // Check R, G, B channels
	                if (newImage[y][4 * x + c] != 0) {
	                    isBlack = false;
	                    break;
	                }
	            }
	            if (isBlack) {
	                newImage[y][4 * x + 3] = 0; // Set alpha to 0 (transparent)
	            }
	        }
	    }
	    return newImage;
	}

	vector<vector<int>> flipImage( vector<vector<int>>& image,int direction) {
	    int height = image.size();
	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	    vector<vector<int>> flippedImage = image;


	    if (direction == 1) {
	        // Flip horizontally
	        for (int y = 0; y < height; ++y) {
	            for (int x = 0; x < width / 2; ++x) {
	                for (int c = 0; c < 4; ++c) {
	                    swap(flippedImage[y][4 * x + c], flippedImage[y][4 * (width - 1 - x) + c]);
	                }
	            }
	        }
	    } else if (direction == 2) {
	        // Flip vertically
	        for (int y = 0; y < height / 2; ++y) {
	            for (int x = 0; x < width; ++x) {
	                for (int c = 0; c < 4; ++c) {
	                    swap(flippedImage[y][4 * x + c], flippedImage[height - 1 - y][4 * x + c]);
	                }
	            }
	        }
	    } else {
	        cerr << "Invalid direction. Use 'horizontal' or 'vertical'." << endl;
	    }

	    return flippedImage;
	}

	vector<vector<int>> normalizeImage(vector<vector<int>>& image) {

	    int height = image.size();

	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)

	    int minValue = std::numeric_limits<int>::max();

	    int maxValue = std::numeric_limits<int>::min();


	    // Find the minimum and maximum pixel values

	    for (int y = 0; y < height; ++y) {

	        for (int x = 0; x < width; ++x) {

	            for (int c = 0; c < 4; ++c) {

	                minValue = min(minValue, image[y][4 * x + c]);
	                maxValue = max(maxValue, image[y][4 * x + c]);
	            }
	        }
	    }


	    // Normalize the pixel values

	    for (int y = 0; y < height; ++y) {

	        for (int x = 0; x < width; ++x) {

	            for (int c = 0; c < 4; ++c) {

	                image[y][4 * x + c] = (image[y][4 * x + c] - minValue) * 255 / (maxValue - minValue);

	            }

	        }

	    }


	    return image;

	}

	vector<vector<int>> rotateImage(vector<vector<int>>& image, int direction) {
	    int height = image.size();
	    int width = image[0].size() / 4; // Since each pixel has 4 components (RGBA)
	 // Initialize rotated image


	
		if (direction==90){
	            // Rotate 90 degrees clockwise
	            vector<vector<int>> rotatedImage = vector<vector<int>>(width, vector<int>(height * 4, 255));
	            for (int y = 0; y < height; ++y) {
	                for (int x = 0; x < width; ++x) {
	                    for (int c = 0; c < 4; ++c) {
	                        rotatedImage[x][4 * y + c] = image[y][4 * x + c];
	                    }
	                }
	            }
	            rotatedImage=flipImageHor(rotatedImage);
	            w = height;
	            h = width;
	            return rotatedImage;
	            }
	
	
		else if (direction==180){
			image=flipImageVert(image);
			image=flipImageHor(image);
			return image;
	    }

		else if (direction==270){
	            // Rotate 90 degrees clockwise
	            vector<vector<int>> rotatedImage = vector<vector<int>>(width, vector<int>(height * 4, 255));
	            for (int y = 0; y < height; ++y) {
	                for (int x = 0; x < width; ++x) {
	                    for (int c = 0; c < 4; ++c) {
	                        rotatedImage[x][4 * y + c] = image[y][4 * x + c];
	                    }
	                }
	            }
	            rotatedImage=flipImageVert(rotatedImage);
	            w = height;
	            h = width;
	            return rotatedImage;
	            }
		else{
			// Rotate 90 degrees clockwise
			cout<<"Enter a proper value"<<endl;
			return image;
	    }

	}

	vector<vector<int>> removefg( vector<vector<int>>& image) {
	    // Sobel operator kernels
	    vector<vector<int>> sobelX = {
	        { -1, 0, 1 },
	        { -2, 0, 2 },
	        { -1, 0, 1 }
	    };

	    vector<vector<int>> sobelY = {
	        { -1, -2, -1 },
	        {  0,  0,  0 },
	        {  1,  2,  1 }
	    };

	    vector<vector<int>> edges(h, vector<int>(w * 4, 0)); // Initialize the edges image
	    // Detect edges using Sobel operator
	    for (int y = 1; y < h - 1; ++y) {
	        for (int x = 1; x < w - 1; ++x) {
	            int gx = 0;
	            int gy = 0;
	            for (int i = -1; i <= 1; ++i) {
	                for (int j = -1; j <= 1; ++j) {
	                    gx += image[y + i][4 * (x + j)] * sobelX[i + 1][j + 1];
	                    gy += image[y + i][4 * (x + j)] * sobelY[i + 1][j + 1];
	                }
	            }
	            int g = sqrt(gx * gx + gy * gy);
	            g = min(max(g, 0), 255);
	            if (g > 128) { // If it's an edge pixel, set to white (foreground)
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = 255;
	                }
	            } else { // Otherwise, set to black (background)
	                for (int c = 0; c < 3; c++) {
	                    edges[y][4 * x + c] = 0;
	                }
	            }
	        }
	    }

	        for (int y = 0; y < edges.size(); ++y) {
	        for (int x = 0; x < edges[0].size(); x += 4) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	        std::vector<int> whitelist;
	        for (int x = 0; x < w; ++x) {
	        int r = edges[h - 2][4 * x];
	        int g = edges[h - 2][4 * x + 1];
	        int b = edges[h - 2][4 * x + 2];
	        int a = edges[h - 2][4 * x + 3];

	        // Check if the pixel is white (R=255, G=255, B=255, A=255)
	        if (r == 255 && g == 255 && b == 255 && a == 255) {
	        	whitelist.push_back(x);
	
	        }
	    }
	    int min =whitelist.front();
	    int max =whitelist.back();

	    edges=flipImageHor(edges);
	            for (int y = 0; y < edges.size(); ++y) {
	        for (int x = 0; x < edges[0].size(); x += 4) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	                for (int x = 0; x < edges[0].size(); x += 4) {
	        for (int  y = 0; y < edges.size(); ++y) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	    edges=flipImageVert(edges);
	
	       for (int x = 0; x < edges[0].size(); x += 4) {
	        for (int  y = 0; y < edges.size(); ++y) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	
	    edges=flipImageVert(edges);
	    edges=flipImageHor(edges);
	    int h = edges.size();         
	    int w = edges[0].size() / 4;  


	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            // Check if the pixel in image a is black (R=0, G=0, B=0, A=255) or white (R=255, G=255, B=255, A=255)
	            if ((edges[y][4 * x] == 0 && edges[y][4 * x + 1] == 0 && edges[y][4 * x + 2] == 0 && edges[y][4 * x + 3] == 255)) {
	                // Replace the pixel in image a with the corresponding pixel from image b
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = image[y][4 * x + c];
	                }
	            }
	        }
	    }
	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            // Check if the pixel in image a is black (R=0, G=0, B=0, A=255) or white (R=255, G=255, B=255, A=255)
	            if ((edges[y][4 * x] == 255 && edges[y][4 * x + 1] == 255 && edges[y][4 * x + 2] == 255 && edges[y][4 * x + 3] == 255)) {
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = 0;
	                }
	            }
	        }
	    }



	    return edges;
	}

	vector<vector<int>> removebg( vector<vector<int>>& image) {
	    // Sobel operator kernels
	    vector<vector<int>> sobelX = {
	        { -1, 0, 1 },
	        { -2, 0, 2 },
	        { -1, 0, 1 }
	    };

	    vector<vector<int>> sobelY = {
	        { -1, -2, -1 },
	        {  0,  0,  0 },
	        {  1,  2,  1 }
	    };

	    vector<vector<int>> edges(h, vector<int>(w * 4, 0)); // Initialize the edges image
	    // Detect edges using Sobel operator
	    for (int y = 1; y < h - 1; ++y) {
	        for (int x = 1; x < w - 1; ++x) {
	            int gx = 0;
	            int gy = 0;
	            for (int i = -1; i <= 1; ++i) {
	                for (int j = -1; j <= 1; ++j) {
	                    gx += image[y + i][4 * (x + j)] * sobelX[i + 1][j + 1];
	                    gy += image[y + i][4 * (x + j)] * sobelY[i + 1][j + 1];
	                }
	            }
	            int g = sqrt(gx * gx + gy * gy);
	            g = min(max(g, 0), 255);
	            if (g > 128) { // If it's an edge pixel, set to white (foreground)
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = 255;
	                }
	            } else { // Otherwise, set to black (background)
	                for (int c = 0; c < 3; c++) {
	                    edges[y][4 * x + c] = 0;
	                }
	            }
	        }
	    }

	        for (int y = 0; y < edges.size(); ++y) {
	        for (int x = 0; x < edges[0].size(); x += 4) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	        std::vector<int> whitelist;
	        for (int x = 0; x < w; ++x) {
	        int r = edges[h - 2][4 * x];
	        int g = edges[h - 2][4 * x + 1];
	        int b = edges[h - 2][4 * x + 2];
	        int a = edges[h - 2][4 * x + 3];

	        // Check if the pixel is white (R=255, G=255, B=255, A=255)
	        if (r == 255 && g == 255 && b == 255 && a == 255) {
	        	whitelist.push_back(x);
	
	        }
	    }
	    int min =whitelist.front();
	    int max =whitelist.back();

	    edges=flipImageHor(edges);
	            for (int y = 0; y < edges.size(); ++y) {
	        for (int x = 0; x < edges[0].size(); x += 4) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	                for (int x = 0; x < edges[0].size(); x += 4) {
	        for (int  y = 0; y < edges.size(); ++y) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	    edges=flipImageVert(edges);
	
	       for (int x = 0; x < edges[0].size(); x += 4) {
	        for (int  y = 0; y < edges.size(); ++y) {
	            // Check if the pixel is not white
	            if (edges[y][x] < 255 || edges[y][x + 1] < 255 || edges[y][x + 2] < 255) {
	                // Set the pixel to black
	                edges[y][x] = 0;
	                edges[y][x + 1] = 0;
	                edges[y][x + 2] = 0;
	                edges[y][x + 3] = 255;

	            }
	            else{
	            break;
	            }
	        }
	    }
	
	    edges=flipImageVert(edges);
	    edges=flipImageHor(edges);
	    int h = edges.size();         
	    int w = edges[0].size() / 4;  


	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            // Check if the pixel in image a is black (R=0, G=0, B=0, A=255) or white (R=255, G=255, B=255, A=255)
	            if ((edges[y][4 * x] == 0 && edges[y][4 * x + 1] == 0 && edges[y][4 * x + 2] == 0 && edges[y][4 * x + 3] == 0)||(edges[y][4 * x] == 255 && edges[y][4 * x + 1] == 255 && edges[y][4 * x + 2] == 255 && edges[y][4 * x + 3] == 255)) {
	                // Replace the pixel in image a with the corresponding pixel from image b
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = image[y][4 * x + c];
	                }
	            }
	        }
	    }
	    for (int y = 0; y < h; ++y) {
	        for (int x = 0; x < w; ++x) {
	            // Check if the pixel in image a is transparent (R=0, G=0, B=0, A=00) or white (R=255, G=255, B=255, A=255)
	            if ((edges[y][4 * x] == 0 && edges[y][4 * x + 1] == 0 && edges[y][4 * x + 2] == 0 && edges[y][4 * x + 3] == 255)) {
	                for (int c = 0; c < 4; ++c) {
	                    edges[y][4 * x + c] = 0;
	                }
	            }
	        }
	    }



	    return edges;
	}

	friend class UI;
};


class Sprite{
protected:
	SDL_Surface *image;
	SDL_Rect rect;
	int o_x,o_y;	// Origin x and Origin y
public:
	Sprite(Uint32 color, int x, int y, int wt = 48, int ht = 64){
		image = SDL_CreateRGBSurface(0, wt, ht, 32, 0, 0, 0, 0 );
		
		SDL_FillRect(image, NULL, color);
		rect=image->clip_rect;
		o_x = 0;
		o_y = 0;
		rect.x = x;
		rect.y = y;
	}
		
	void update(){
		// Can be overridden
	}
	
	void draw(SDL_Surface *dest){	// dest is destination
		SDL_BlitSurface(image, NULL, dest, &rect);
	}
	
	SDL_Surface* getImage() const{
		return image;
	}
	
	bool operator==(const Sprite &other) const{		//creates an operator to compare two SDL_Surfaces
		return(image == other.getImage());
	}
};

class SpriteGroup{
private:
	vector <Sprite*> sprites;		// Creates a vector named sprites that contains pointers to the objects of the class Sprite
	int sprites_size;
public:
	SpriteGroup copy(){
		SpriteGroup new_group;
		for(int i=0;i<sprites_size;i++){
			new_group.add(sprites[i]);		// adds sprites to the new_group object of SpriteGroup class
		}
		return new_group;
	}
	
	void add(Sprite* sprite){
		sprites.push_back(sprite);		// adds a sprite at the end of the vector
		sprites_size=sprites.size();	// stores the size of the vector in a variable so that we don't have to keep calling the function
	}
	
	void remove(Sprite sprite_object){
		for(int i=0; i<sprites_size;i++){
			if(*sprites[i] == sprite_object){		// checks for the position of the sprite to be removed
				sprites.erase(sprites.begin()+i);	// removes the required sprite
			}
		}
		sprites_size = sprites.size();
	}
	
	bool has(Sprite sprite_object){
		for(int i=0; i<sprites_size;i++){
			if(*sprites[i] == sprite_object){
				return true;
			}
		}
		return false;
	}
	
	void update(){
		if(!sprites.empty()){
			for(int i=0; i<sprites_size;i++){
				sprites[i]->update();
			}
		}
	}
	
	void draw(SDL_Surface *dest){
		if(!sprites.empty()){
			for(int i=0; i<sprites_size;i++){
				sprites[i]->draw(dest);
			}
		}
	}
	
	void empty(){
		sprites.clear();
		sprites_size=sprites.size();
	}

	int size(){
		return sprites_size;
	}
	
	vector <Sprite*> get_sprites(){
		return sprites;		// to get sprites
	}
	
};

class Block: public Sprite{
public:
	SDL_Surface *l_image=NULL;		// variable for loading an image as a SDL_Surface
	Block(Uint32 color, int x, int y, int wt, int ht): Sprite(color,x,y,wt,ht){	// runs the constructor for Sprite class with same arguments as Block construstor
		update_properties();
	}
	void update_properties(){
		o_x=0;
		o_y=0;

		set_position(rect.x,rect.y);
	}
	void set_position(int x, int y){
		rect.x = x - o_x;
		rect.y = y - o_y;
	}
	void set_image(const char filename[]=NULL){
		if(filename!=NULL){
			l_image=IMG_Load(filename);		// loads any image type

			if(l_image!=NULL){
				image=l_image;
				int old_x=rect.x, old_y=rect.y;
				rect=image->clip_rect;
				rect.x=old_x;
				rect.y=old_y;
				update_properties();
			}
		}
	}
};

class UI{
private:
	SDL_Window *window = NULL;		// Declare a Pointer
	SDL_Surface *screen = NULL, *texts = NULL, *texts0 = NULL, *text = NULL, *text0 = NULL, *mb0 = NULL, *textt = NULL, *mb00 = NULL, *mb1 = NULL, *mb2 = NULL, *mb3 = NULL, *mb4 = NULL, *mb5 = NULL, *mb6 = NULL;
	Uint32 black = 0, white = 0, blue = 0, green = 0, red = 0;
	SDL_Rect rect, rect0, rect1, rectu, dstrect, rectm0, rectt, rectm00, rectm1, rectm2, rectm3, rectm4, rectm5, rectm6, rectm7;
	TTF_Font *aloe = NULL, *sten = NULL;
	Mix_Music *music = NULL, *click = NULL;
	int evnt = 0, c = 0;
	string s;
public:
	UI(){
		SDL_Init( SDL_INIT_EVERYTHING );	// Initializes everything in SDL2

		// Create an application window with the following settings:
		window = SDL_CreateWindow( "Image Processing in C++",	// Window title
									SDL_WINDOWPOS_UNDEFINED,	// Inital x position
									SDL_WINDOWPOS_UNDEFINED,	// Inital y position
									window_width,	// width, in pixels
									window_height,	// height, in pixels
									SDL_WINDOW_SHOWN	// Flags
									);

		// Check if the window was made successfully
		if(window == NULL){
			// If the window could not be made
			cout << "Could not create window:\n" << SDL_GetError() << endl;\
		}

		screen = SDL_GetWindowSurface( window );
		//	SDL_FillRect(screen, NULL, white);		// creates a rectangle to fill the screen with white

		colorInit();		// Initializes a general color pallete
		TTF_Init();			// Initializes the TTF library

	start:

		SDL_Surface *background = IMG_Load("backg.jpg");
		// Fill the screen with the image
    	dstrect = {0, 0, window_width, window_height};		// x,y,w,h
    	SDL_BlitScaled(background, NULL, screen, &dstrect);		// BlitScaled is used to scaled one surface to another and overlap it

    	// Update the screen
    	SDL_UpdateWindowSurface(window);

		//Block block(blue, window_width/2, window_height/2,48,64);
		Block up(white, (window_width/2) - 270, (window_height/2) - 150 ,539,86);
		up.set_image("upld.png");		// sends the image to be loaded


		Uint32 arr[3]={red,green,blue};
		srand(time(0));		// seeds the random number generator

		SpriteGroup active_sprites;
		//active_sprites.add(&block);
		active_sprites.add(&up);
		active_sprites.draw(screen);

		// If the window is open, we enter the program loop (see SDL_PollEvent)

		// SDL_Delay(3000);		// Pause execution for 3000 milliseconds

		// Initialize mixer
		// Mix_OpenAudio(frequency, format, channels, chunk_size)
		Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 4096);	// Default frequency set by mixer is 22050
		/*
		Mix_Chunk *sound = NULL;	// to be overrided
		sound = Mix_LoadWAV("sample.wav");
		// Mix_PlayChannel(channel, chunk, loop)
		Mix_PlayChannel(-1, sound, 1);
		*/
		music = Mix_LoadMUS("bgm.mp3");
		click = Mix_LoadMUS("click0.mp3");
		// Mix_PlayMusic(music, loop)
		//Mix_PlayMusic(music, 0);	// loop can be either 0 or -1



		evnt=0;
		SDL_FreeSurface(screen);
		setup();
		refresh();
		Uint32 starting_tick;
		SDL_Event event;
		bool running = true;
		vector<vector<int>> im, out;
		// int x, y, w, h;
		Util obj;

		while(running){
			starting_tick = SDL_GetTicks();
			while(SDL_PollEvent(&event)){
				if(event.type == SDL_QUIT){
					running = false;
					break;
				} else{
					switch (evnt){
						case 0:
							if (event.type == SDL_MOUSEBUTTONDOWN){
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x >= 0 && x <= 1600 && y >= 0 && y <= 900){
									Mix_PlayMusic(click, 0);
									evnt = 1;
								}
							}
							break;
						case 1:
							s = upload();
							im = inp(s.c_str());
							evnt = 2;
						case 2:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );

    						SDL_BlitScaled(background, NULL, screen, &dstrect);
							pmenu();
							refresh();

							if (event.type == SDL_MOUSEBUTTONDOWN){
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=1167 && x<1277 && y>=372 && y<497){
									Mix_PlayMusic(click, 0);
									evnt+=1;
								} else if (x>=1160 && x<1277 && y>=588 && y<703){
									Mix_PlayMusic(click, 0);
									goto start;
								}
							}
							break;
						case 3:
							menu:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );

    						SDL_BlitScaled(background, NULL, screen, &dstrect);
							menu();
							refresh();
							//mouseCoord();
							if (event.type == SDL_MOUSEBUTTONDOWN){
								Mix_PlayMusic(click, 0);
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=203 && y>=350 && x<=554 && y<=606){
									Mix_PlayMusic(click, 0);
									evnt=4;
								} else if (x>=626 && y>=350 && x<=977 && y<=606){
									Mix_PlayMusic(click, 0);
									evnt=5;
								} else if (x>=window_width - 553 && y>=350 && x<=window_width - 202 && y<=606){
									Mix_PlayMusic(click, 0);
									evnt=6;
								}
							}
							break;
						case 4:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );
							SDL_BlitScaled(background, NULL, screen, &dstrect);
							fx();
							refresh();
							//mouseCoord();
							if (event.type == SDL_MOUSEBUTTONDOWN){
								Mix_PlayMusic(click, 0);
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=20 && x<145 && y>=15 && y<90){
                            	    evnt=0;
                            	    goto start;
                            	} else if (x>=202 && y>=370 && x<=568 && y<=430){
									out = obj.enhanceImage(im, 20, 10);
									cout << "Image has been enhanced" << endl;
                            	    op(out);
                            	    evnt=7;
                            	} else if (x>=626 && y>=370 && x<=992 && y<=430){
									out = obj.sharpenImage(im, out);
									cout << "Image has been sharpened" << endl;
									op(out);
									evnt=7;
								} else if (x>=1050 && y>=370 && x<=1416 && y<=430){
									out = obj.normalizeImage(im);
									cout << "Image has been normalized" << endl;
									op(out);
									evnt=7;
								} else if (x>=390 && y>=590 && x<=741 && y<=750){
									out = obj.scaleImage(im, 0.5, 0.5);
									cout << "Image has been scaled" << endl;
                            	    op(out);
                            	    evnt=7;
                            	} else if (x>=844 && y>=590 && x<=1195 && y<=750){
									out = obj.applyBlur(im,7);
									cout << "Image has been blurred" << endl;
                            	    op(out);
                            	    evnt=7;
                            	}
							}
							break;
						case 5:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );

							SDL_BlitScaled(background, NULL, screen, &dstrect);
							oper();
							refresh();
							if (event.type == SDL_MOUSEBUTTONDOWN){
								Mix_PlayMusic(click, 0);
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=390 && y>=370 && x<=741 && y<=530){
									out = obj.flipImageHor(im);
                                    cout << "Image has been flipped" << endl;
                                    op(out);
                                    evnt=7;
								} else if (x>=844 && y>=370 && x<=1195 && y<=530){
									out = obj.rotateImage(im, 90);
                                    cout << "Image has been rotated" << endl;
                                    op(out);
                                    evnt=7;
                                } else if (x>=20 && x<145 && y>=15 && y<90){
                                    evnt=0;
                                    goto start;
                                } else if (x>=390 && y>=590 && x<=741 && y<=750){
									out = obj.removebg(im);
                                    cout << "Background has been removed from the Image" << endl;
                                    op(out);
                                    evnt=7;
                                } else if (x>=844 && y>=590 && x<=1195 && y<=750){
									out = obj.removefg(im);
                                    cout << "Foreground has been removed from the Image" << endl;
                                    op(out);
                                    evnt=7;
                                }
							}
							break;
						case 6:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );

							SDL_BlitScaled(background, NULL, screen, &dstrect);
							filters();
							refresh();
							//mouseCoord();
							if (event.type == SDL_MOUSEBUTTONDOWN){
								Mix_PlayMusic(click, 0);
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=390 && y>=370 && x<=741 && y<=530){
									out = obj.convertToGrayscale(im);
                                    cout << "Image has been converted to grayscale" << endl;
                                    op(out);
                                    evnt=7;
								} else if (x>=844 && y>=370 && x<=1195 && y<=530){
									out = obj.pixelateImage(im, 10);
                                    cout << "Image has been converted to pixelate" << endl;
                                    op(out);
                                    evnt=7;
                                } else if (x>=20 && x<145 && y>=15 && y<90){
                                    evnt=0;
                                    goto start;
                                }else if (x>=390 && y>=590 && x<=741 && y<=750){
									out = obj.Canvas(im);
                                    cout << "Image has been converted to a Black and White canvas" << endl;
                                    op(out);
                                    evnt=7;
                                } else if (x>=844 && y>=590 && x<=1195 && y<=750){
									out = obj.Saturate(im);
                                    cout << "Image has been Retro-fied" << endl;
                                    op(out);
                                    evnt=7;
                                }
							}
							break;
						case 7:
							SDL_FreeSurface(screen);
							screen = SDL_GetWindowSurface( window );

    						SDL_BlitScaled(background, NULL, screen, &dstrect);
							omenu("output.png");
							refresh();

							if (event.type == SDL_MOUSEBUTTONDOWN){
								int x, y;
								SDL_GetMouseState(&x, &y);
								if (x>=1167 && x<1277 && y>=372 && y<497){
									Mix_PlayMusic(click, 0);
									goto end;
								} else if (x>=1160 && x<1277 && y>=588 && y<703){
									Mix_PlayMusic(click, 0);
									evnt=3;
									goto menu;
								}
							}
							break;
					}
				
				}
			}
			// SDL_GetWindowPosition(window, &x, &y);// gets the window position
			// cout << x << "," << y  << endl;
			cap_framerate( starting_tick );
		}
	end:
	cout << "Thank you for using PNG Editor!" << endl;
	}
	void setup(){
		aloe=TTF_OpenFont("Aloevera.ttf",84);
		sten=TTF_OpenFont("stencil.ttf",24);
		texts=TTF_RenderText_Solid(aloe, "Image Processing", {0,0,0});
		rect0 = {482, 147, 700, 100};		// x, y, w, h

		text=TTF_RenderText_Solid(aloe, "Image Processing", {225,225,225});
		rect = {485, 150, 700, 100};

		texts0=TTF_RenderText_Solid(sten, "Upload and work on any PNG Image", {225,225,225});
		rect1 = {600, 850, 700, 100};

		text0=TTF_RenderText_Solid(sten, "Upload a PNG Image", {225,225,225});
		rectu= {670,  (window_height/2) - 25 , 700, 100};

		SDL_BlitSurface(text0,NULL,screen,&rectu);

		// Mix_FadeInMusic(music, loop, ms)
		Mix_FadeInMusic(music, 0, 2000);		// loop can be either 0 or -1
	}
	void refresh(){
		SDL_BlitSurface(texts,NULL,screen,&rect0);
		SDL_BlitSurface(text,NULL,screen,&rect);	
		SDL_BlitSurface(texts0,NULL,screen,&rect1);

		SDL_UpdateWindowSurface(window);
	}
	void close(){
		TTF_Quit();
		Mix_CloseAudio();				// Shutdown and cleanup the mixer
		SDL_DestroyWindow(window);		// Close and destroy the window
		SDL_Quit();	
	}
	void colorInit(){
		//white = SDL_MapRGBA( screen->format, 255, 255, 255,100);	
		black 	= SDL_MapRGB(screen->format, 0,0, 0);
		white	= SDL_MapRGB(screen->format, 255, 255, 255);	// White is assigned a value of (255,255,255)
		blue 	= SDL_MapRGB(screen->format, 0, 0, 255);
		green	= SDL_MapRGB(screen->format, 0, 255, 0);
		red 	= SDL_MapRGB(screen->format, 255, 0, 0);
	}
	string upload(){
		system("/home/mrxtreme07/location");
		ifstream inFile("location.txt");
		string s;
		getline(inFile, s);
		return s;
	}
	void pmenu(){
		mb1 = IMG_Load(s.c_str());
		if(w<=window_width - 800 && h<=450){
			rectm1 = {300 + ((window_width - 800 - w)/2),  300 + ((450 - h)/2) ,w ,h};
			SDL_BlitSurface(mb1, NULL, screen, &rectm1);
		} else if(w>window_width - 800 && h<=450){
			rectm1 = {300 + ((window_width - 800 - w)/2), 300 + ((450 - h)/2) ,window_width - 800 ,h};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		} else if(w<=window_width - 800 && h>450){
			rectm1 = {300, 300 ,w ,450};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		} else if(w>window_width - 800 && h>450){
			rectm1 = {300, 300 ,window_width - 800 ,450};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		}

		mb2 = IMG_Load("continue.png");
		rectm2 = {1100, (window_height/2)-128,256, 256};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("retry.png");
		rectm3 = {1065, (window_height/2)+70,310, 295};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);
	}
	void menu(){
		mb0 = IMG_Load("home.png");
		rectm0 = {20, 15, 125, 25};
		SDL_BlitScaled(mb0, NULL, screen, &rectm0);

		mb1 = IMG_Load("mb1.png");
    	rectm1 = {203, 350, 351, 256};
		SDL_BlitScaled(mb1, NULL, screen, &rectm1);

		mb2 = IMG_Load("mb2.png");
    	rectm2 = {626, 350, 351, 256};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("mb3.png");
    	rectm3 = {window_width - 553, 350, 351, 256};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);
	}
	void fx(){
		textt=TTF_RenderText_Solid(sten, "Effects", {225,225,225});
		rectt= {485, 250, 700, 100};
		SDL_BlitSurface(textt,NULL,screen,&rectt);

		mb0 = IMG_Load("home.png");
		rectm0 = {20, 15, 125, 25};
		SDL_BlitScaled(mb0, NULL, screen, &rectm0);

		mb00 = IMG_Load("arrow.png");
		rectm00 = {20, 35, 125, 25};
		SDL_BlitScaled(mb00, NULL, screen, &rectm00);

		mb1 = IMG_Load("Enhance.png");
    	rectm1 = {202, 370, 366, 160};
		SDL_BlitScaled(mb1, NULL, screen, &rectm1);

		mb2 = IMG_Load("Sharpen.png");
    	rectm2 = {617, 370, 366, 160};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("Normalize.png");
    	rectm3 = {1032, 370, 366, 160};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);

		mb4 = IMG_Load("Scale.png");
		rectm4 = {390, 590, 366, 160};
		SDL_BlitScaled(mb4, NULL, screen, &rectm4);

		mb5 = IMG_Load("Blur.png");
		rectm5 = {844, 590, 366, 160};
		SDL_BlitScaled(mb5, NULL, screen, &rectm5);
	}
	void filters(){
		textt=TTF_RenderText_Solid(sten, "Filters", {225,225,225});
		rectt= {485, 250, 700, 100};
		SDL_BlitSurface(textt,NULL,screen,&rectt);

		mb0 = IMG_Load("home.jpg");
		rectm0 = {20, 15, 125, 25};
		SDL_BlitScaled(mb0, NULL, screen, &rectm0);

		mb00 = IMG_Load("arrow.png");
		rectm00 = {20, 35, 125, 25};
		SDL_BlitScaled(mb00, NULL, screen, &rectm00);

		mb1 = IMG_Load("grayscale.png");
    	rectm1 = {390, 370, 351, 160};
		SDL_BlitScaled(mb1, NULL, screen, &rectm1);

		mb2 = IMG_Load("pixelate.png");
    	rectm2 = {844, 370, 351, 160};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("canvas.png");
    	rectm3 = {390, 590, 351, 160};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);

		mb4 = IMG_Load("fburn.png");
		rectm4 = {844, 590, 351, 160};
		SDL_BlitScaled(mb4, NULL, screen, &rectm4);
	}
	void oper(){
		textt=TTF_RenderText_Solid(sten, "Operations", {225,225,225});
		rectt= {485, 250, 700, 100};
		SDL_BlitSurface(textt,NULL,screen,&rectt);
		
		mb0 = IMG_Load("home.jpg");
		rectm0 = {20, 15, 125, 25};
		SDL_BlitScaled(mb0, NULL, screen, &rectm0);

		mb00 = IMG_Load("arrow.png");
		rectm00 = {20, 35, 125, 25};
		SDL_BlitScaled(mb00, NULL, screen, &rectm00);

		mb1 = IMG_Load("Flip.png");
    	rectm1 = {390, 370, 351, 160};
		SDL_BlitScaled(mb1, NULL, screen, &rectm1);

		mb2 = IMG_Load("rotate.png");
    	rectm2 = {844, 370, 351, 160};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("removebg.png");
    	rectm3 = {390, 590, 351, 160};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);

		mb4 = IMG_Load("removefg.png");
		rectm4 = {844, 590, 351, 160};
		SDL_BlitScaled(mb4, NULL, screen, &rectm4);
	}
	void omenu(string ot){
		mb1 = IMG_Load(ot.c_str());
		if(w<=window_width - 800 && h<=450){
			rectm1 = {300 + ((window_width - 800 - w)/2),  300 + ((450 - h)/2) ,w ,h};
			SDL_BlitSurface(mb1, NULL, screen, &rectm1);
		} else if(w>window_width - 800 && h<=450){
			rectm1 = {300 + ((window_width - 800 - w)/2), 300 + ((450 - h)/2) ,window_width - 800 ,h};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		} else if(w<=window_width - 800 && h>450){
			rectm1 = {300, 300 ,w ,450};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		} else if(w>window_width - 800 && h>450){
			rectm1 = {300, 300 ,window_width - 800 ,450};
			SDL_BlitScaled(mb1, NULL, screen, &rectm1);
		}

		mb2 = IMG_Load("continue.png");
		rectm2 = {1100, (window_height/2)-128,256, 256};
		SDL_BlitScaled(mb2, NULL, screen, &rectm2);

		mb3 = IMG_Load("retry.png");
		rectm3 = {1065, (window_height/2)+70,310, 295};
		SDL_BlitScaled(mb3, NULL, screen, &rectm3);
	}
	void mouseCoord(){
		int x, y;
		SDL_GetMouseState(&x, &y);
		cout << x << "," << y << endl;
	}
	void cap_framerate( Uint32 starting_tick){
		if((1000/fps)>SDL_GetTicks() - starting_tick){
			SDL_Delay(1000/fps-(SDL_GetTicks() - starting_tick));
		}
	}
	~UI(){
		close();
	}
};

int main(int argc, char *argv[] ){
	UI obj;
	return 0;
}
