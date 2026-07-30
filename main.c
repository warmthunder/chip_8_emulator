#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>

#define stack_length 16

// stack with basic push and pop
typedef struct{
    uint16_t stk[stack_length];
    int top;
}stackk ;

void stk_push(stackk* stack, uint16_t value){
    if(stack->top >= stack_length-1){
        printf("%s","Stack is full");
        return;
    }
    stack->stk[stack->top] = value;
    stack->top += 1;
}

uint16_t stk_pop(stackk* stack){
    if(stack->top == 0)
        return -1;
    stack->top -=1;
    return stack->stk[stack->top+1];
}

// ch8 components
typedef struct{
    uint8_t registers[16];
    uint8_t memory[4096];
    uint16_t index_register;
    uint16_t program_counter;
    stackk stack;
    uint8_t stack_pointer;
    uint8_t delay_timer;
    uint8_t sound_timer;
    // bool keypress[16];
    bool display[64*32];
}chip8;

// sdl boilerplate
SDL_Window *window;
SDL_Renderer *renderer;

int render(SDL_Renderer* renderer){
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);
    return 0;
    }


void update(){

}

int main(){
SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("pong", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000,1000, SDL_WINDOW_RESIZABLE);

    // just to check if window creation failed
    if(window == NULL)
{
    printf("Window error: %s\n", SDL_GetError());
    return 1;
}
// for me it doesnt show window until something is drawn on it
    SDL_Renderer* renderer =
    SDL_CreateRenderer(window, -1, 0);
    SDL_RenderPresent(renderer);
    bool quit = false;
    SDL_Event event;
    while(!quit){

        // input
        while( SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        update();
        render(renderer);
    }

    return 0;
    return 0;
}