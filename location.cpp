#include <iostream>
#include <fstream>
#include <SDL2/SDL.h>
#include "SDL_image.h"
#include <gtk/gtk.h>
using namespace std;

const char* filename = NULL;

// Callback function to handle button click event
void on_button_clicked(GtkWidget *widget, gpointer data) {
    // Create a GTK file chooser dialog
    GtkWidget *dialog;
    dialog = gtk_file_chooser_dialog_new("Open File", NULL, GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);

    // Run the file chooser dialog
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        // Get the selected file path
        const char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        std::cout << "Selected file: " << filename << std::endl;
        ofstream outFile("location.txt");
        outFile << filename;
        outFile.close();

        // Close the SDL window
        SDL_Event quitEvent;
        quitEvent.type = SDL_QUIT;
        SDL_PushEvent(&quitEvent);
    } else {
        std::cout << "No file selected" << std::endl;
    }

    // Cleanup GTK
    gtk_widget_destroy(dialog);
}

int main() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL initialization failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize GTK
    gtk_init(NULL, NULL);

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("GTK File Chooser", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 320, 240, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create SDL renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Create SDL button texture
    SDL_Surface* buttonSurface = IMG_Load("Designer0.jpeg"); // Replace "button.bmp" with your button image
    if (!buttonSurface) {
        std::cerr << "Failed to load button image: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    SDL_Texture* buttonTexture = SDL_CreateTextureFromSurface(renderer, buttonSurface);
    SDL_FreeSurface(buttonSurface);
    if (!buttonTexture) {
        std::cerr << "Failed to create button texture: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Event loop
    bool quit = false;
    SDL_Event event;
    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_MOUSEBUTTONDOWN) {
                // On mouse click event, check if it's within button area
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (x >= 0 && x <= 320 && y >= 0 && y <= 240) {
                    // Call button click callback function
                    on_button_clicked(NULL, NULL);
                }
            }
        }

        // Render button texture
        SDL_RenderClear(renderer);
        SDL_Rect dstRect = {0, 0, 320, 240};
        SDL_RenderCopy(renderer, buttonTexture, nullptr, &dstRect); // Pass nullptr as the source
        SDL_RenderPresent(renderer);
    }

    // Cleanup SDL
    SDL_DestroyTexture(buttonTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
