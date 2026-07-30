#include <SDL2/SDL_render.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <sys/types.h>

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
    stack->top ++;
}

uint16_t stk_pop(stackk* stack){
    if(stack->top == 0)
        return -1;
    stack->top--;
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
    uint32_t display[64*32];
}chip8;

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
bool quit = false;
int movement[4] = {0,0,0,0};

static void sdl_process_event(SDL_Event *event){
   switch(event->type){
        case SDL_KEYDOWN:
            SDL_Keycode key = event->key.keysym.sym;
            if(key==SDLK_s)
                movement[1] = +10;
            if(key==SDLK_w)
                movement[1] = -10;
            break;
         case SDL_KEYUP:
            key = event->key.keysym.sym;
            if(key==SDLK_s)
                movement[1] = 0;
            if(key==SDLK_w)
                movement[1]  = 0;
            break;
        case SDL_QUIT:{
            quit = true;
        }
    }
}

int render(SDL_Renderer* renderer, SDL_Texture* texture, chip8* instance){
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);

    SDL_UpdateTexture(texture, NULL, instance->display, 64*sizeof(uint32_t));
    SDL_RenderCopy(renderer,texture,NULL,NULL);
    SDL_RenderPresent(renderer);
    return 0;
    }


void decode(uint32_t opcode, chip8* instance){
uint8_t x = (opcode & 0x0F00) >> 8;
uint8_t y = (opcode & 0x00F0) >> 4;
uint16_t sum = instance->registers[x] + instance->registers[y];
uint8_t vF = 0xF;

switch(opcode){
    case 0x00E0:
        memset(instance->display, 0, sizeof(instance->display)); 
        break;
    case 0x00EE:
        instance->program_counter = stk_pop(&instance->stack);
        instance->stack_pointer--;
        break;
    break;
}
switch(opcode & 0xF000){
    case 0x1000:
        instance->program_counter = opcode & 0x0FFF;
        break;
    case 0x2000:
        stk_push(&instance->stack, instance->program_counter);
        instance->program_counter = opcode & 0x0FFF;
        break;
    case 0x3000:
        if(instance->registers[x] == y)
        // skip next instruction if condition is met
            instance->program_counter += 2;
        break;
    case 0x4000:
        if(instance->registers[x] != y)
            instance->program_counter += 2;
        break;
    case 0x5000:
        if(instance->registers[x] == instance->registers[y])
            instance->program_counter += 2;
        break;
    case 0x6000:
        instance->registers[x] = opcode & 0x00FF;
        break;
    case 0x7000:
        instance->registers[x] += opcode & 0x00FF;
        break;
    case 0x8000:
        instance->registers[x] = instance->registers[y];
        break;
        switch(opcode & 0x000F){
        case 0x8001:
            instance->registers[x] = instance->registers[x] | instance->registers[y];
            break;
        case 0x8002:
            instance->registers[x] = instance->registers[x] & instance->registers[y];
            break;
        case 0x8003:
            instance->registers[x] = instance->registers[x] ^ instance->registers[y];
            break;
        case 0x8004:
            if(sum>0xFF){
                instance->registers[x] = sum & 0xFF;
                instance->registers[vF] = 1;
            }
            else {
                instance->registers[x] =sum;
                instance->registers[vF] = 0;
            }
        case 0x8005:
            if(instance->registers[x]>=instance->registers[y]){
                instance->registers[x] = instance->registers[x] - instance->registers[y];
                instance->registers[vF] = 0;
            }
            else{
                instance->registers[vF] = 1;
            }
            break;
        case 0x8006:
            if(instance->registers[x]%2==0){
                instance->registers[x] = instance->registers[x]>>1;
                instance->registers[vF] = 0;
            }
            else{
                instance->registers[x] = instance->registers[x]>>1;
                instance->registers[vF] = 1;
            }
            break;
        case 0x8007:
                if(instance->registers[y]>instance->registers[x]){
                    instance->registers[vF] = 1;
                }
                else{
                    instance->registers[x]-=instance->registers[y];
                }
            break;
        case 0x800E:
            if(instance->registers[x]%2==0){
                instance->registers[x] = instance->registers[x]<<1;
                instance->registers[vF] = 0;
            }
            else{
                instance->registers[x] = instance->registers[x]<<1;
                instance->registers[vF] = 1;
            }
            break;
    }
    break;  
}

}

void update(chip8* instance){
    
}

size_t loadromtomem(chip8* instance, char* romname){
    FILE *file = fopen(romname, "rb");
     if (file == NULL) {
        perror("Error opening file");
        return 0;
    }
    size_t bytesread = fread(
    &instance->memory[0x200],
    1,
    sizeof(instance->memory) - 0x200,
    file
);
    fclose(file);
    return bytesread;
}

int main(){
    chip8 instance = {0};
    size_t bytes = loadromtomem(&instance, "chp8logo.ch8");
    memset(instance.display, 0, sizeof(instance.display)); 
    // instance.display[266] = 0xFFFFFFFF;

    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1000,1000, SDL_WINDOW_RESIZABLE);

    // just to check if window creation failed
    if(window == NULL)
    {
    printf("Window error: %s\n", SDL_GetError());
    return 1;
    }

    SDL_Renderer* renderer =
    SDL_CreateRenderer(window, -1, 0);
    SDL_RenderPresent(renderer);
    
    SDL_Texture* screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 64, 32);

    SDL_Event event;
    while(!quit){

        // input
        while( SDL_PollEvent(&event)){
            sdl_process_event(&event);
        }
        update(&instance);
        render(renderer, screen, &instance);
    }

    return 0;
}