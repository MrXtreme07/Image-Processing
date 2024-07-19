#include <iostream>
#include <fstream>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_mixer.h"
#include "SDL_ttf.h"
#include <vector>
#include <gtk/gtk.h>
#include <cstdlib>
using namespace std;

#define window_width 1600
#define window_height 900

#define fps 60

void cap_framerate( Uint32 starting_tick){
	if((1000/fps)>SDL_GetTicks() - starting_tick){
		SDL_Delay(1000/fps-(SDL_GetTicks() - starting_tick));
	}
}


class Sprite{
protected:
	SDL_Surface *image;
	SDL_Rect rect;
	int o_x,o_y;	// Origin x and Origin y
public:
	Sprite(Uint32 color, int x, int y, int w = 48, int h = 64){
		image = SDL_CreateRGBSurface(0, w, h, 32, 0, 0, 0, 0 );
		
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
	vector <Sprite*> sprites;		// Creates a vector named sprite that contains pointers to the objects of the class Sprite
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
	Block(Uint32 color, int x, int y, int w = 48, int h = 64): Sprite(color,x,y,w,h){	// runs the constructor for Sprite class with same arguments as Block construstor
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
			SDL_Surface *l_image=NULL;		// variable for loading an image as a SDL_Surface
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

// Callback function to handle button click event
void on_button_clicked() {
    // Create a GTK file chooser dialog
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

    // Run the file chooser dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Get the selected file path
        gchar *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::cout << "Selected file: " << filename << std::endl;
        //	
		g_free(filename);

    	// Cleanup GTK
    	gtk_widget_destroy(dialog);

        // Close the SDL window
        /*SDL_Event quitEvent;
        quitEvent.type = SDL_QUIT;
        SDL_PushEvent(&quitEvent);*/
    } else {
        std::cout << "No file selected" << std::endl;
    }
}

int main(int argc, char *argv[] ){

	SDL_Init( SDL_INIT_EVERYTHING );	// Initializes everything in SDL2
	
	SDL_Window *window = NULL;		// Declare a Pointer
	
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
		cout << "Could not create window:\n" << SDL_GetError() << endl;
		return 1;
	}
	
	SDL_Surface *screen = SDL_GetWindowSurface( window );
	//Uint32 white = SDL_MapRGBA( screen->format, 255, 255, 255, 255);
	Uint32 white = SDL_MapRGB( screen->format, 255, 255, 255);	// White is assigned a value of (255,255,255)
	Uint32 blue = SDL_MapRGB( screen->format, 0, 0, 255);
	Uint32 green = SDL_MapRGB( screen->format, 0, 255, 0);
	Uint32 red = SDL_MapRGB( screen->format, 255, 0, 0);
	//	SDL_FillRect(screen, NULL, white);		// creates a rectangle to fill the screen with white

	SDL_Surface *background = IMG_Load("bg0.jpeg");
	// Fill the screen with the image
    SDL_Rect dstrect = {0, 0, window_width, window_height};
    SDL_BlitScaled(background, NULL, screen, &dstrect);

    // Update the screen
    SDL_UpdateWindowSurface(window);

	system("/home/mrxtreme07/location");
	ifstream inFile("location.txt");
	string s;
	getline(inFile, s);
	const char* filename = s.c_str();
	cout << filename << endl;

	Block block(red, window_width/2, window_height/2);
	//block.set_image("output.png");		// sends the image to be loaded


	Uint32 arr[3]={red,green,blue};
	srand(time(0));		// seeds the random number generator

	SpriteGroup active_sprites;
	active_sprites.add(&block);
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

	Mix_Music *music = NULL;
	music = Mix_LoadMUS("sample0.mp3");
	// Mix_PlayMusic(music, loop)
	//Mix_PlayMusic(music, 0);	// loop can be either 0 or -1

	// Mix_FadeInMusic(music, loop, ms)
	// Mix_FadeInMusic(music, 0, 2000);	// loop can be either 0 or -1

	// SDL ttf
	TTF_Init();
	TTF_Font* font=TTF_OpenFont("stencil.ttf",64);
	SDL_Surface* text=TTF_RenderText_Solid(font, "Image Processing", {0,0,0});
	SDL_Rect rect = {(window_width/2, window_height/2, 100, 100)};	// x, y, w, h
	SDL_BlitSurface(text,NULL,screen,&rect);

	Uint32 starting_tick;
	SDL_Event event;
	bool running = true;
	
	// int x, y, w, h;
	
	while(running){
		SDL_UpdateWindowSurface(window);
		starting_tick = SDL_GetTicks();
		while(SDL_PollEvent(&event)){
			if(event.type == SDL_QUIT){
				running = false;
				break;
			}
		}
		// SDL_GetWindowPosition(window, &x, &y);// gets the window position
		// cout << x << "," << y  << endl;
		cap_framerate( starting_tick );
	}
	TTF_Quit();
	Mix_CloseAudio();	// Shutdown and cleanup the mixer
	SDL_DestroyWindow(window);		// Close and destroy the window
	SDL_Quit();			// Safely ends the program
	return 0;
}
