#include <SDL2/SDL_render.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdlib.h>
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
    uint16_t I;
}chip8;

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
bool quit = false;
int movement[4] = {0,0,0,0};
bool keys[16];

static void sdl_process_event(SDL_Event *event){
   switch(event->type){
    SDL_Keycode key = event->key.keysym.sym;
        case SDL_KEYDOWN:
            switch(key){
                case SDLK_s:
                    keys[0] = true;
                    break;
                case SDLK_w:
                    keys[1] = true;
                    break;
                case SDLK_a:
                    keys[2] = true;
                    break;
                case SDLK_d:
                    keys[3] = true;
                    break;
            break;
            }
            // if(key==SDLK_s)
            //     movement[1] = +10;
            // if(key==SDLK_w)
            //     movement[1] = -10;
            // break;
         case SDL_KEYUP:
               switch(key){
                case SDLK_s:
                    keys[0] = false;
                    break;
                case SDLK_w:
                    keys[1] = false;
                    break;
                case SDLK_a:
                    keys[2] = false;
                    break;
                case SDLK_d:
                    keys[3] = false;
                    break;
            break;
            }

            // if(key==SDLK_s)
            //     movement[1] = 0;
            // if(key==SDLK_w)
            //     movement[1]  = 0;
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
uint8_t n3 = opcode & 0x0FFF;
uint8_t n2 = opcode & 0x00FF;

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
        switch(opcode & 0x000F){
            case 0x0:
                instance->registers[x] = instance->registers[y];
                break;
            case 0x1:
                instance->registers[x] = instance->registers[x] | instance->registers[y];
                break;
            case 0x2:
                instance->registers[x] = instance->registers[x] & instance->registers[y];
                break;
            case 0x3:
                instance->registers[x] = instance->registers[x] ^ instance->registers[y];
                break;
            case 0x4:
                if(sum>0xFF){
                    instance->registers[x] = sum & 0xFF;
                    instance->registers[vF] = 1;
                }
                else {
                    instance->registers[x] =sum;
                    instance->registers[vF] = 0;
                }
            case 0x5:
                if(instance->registers[x]>=instance->registers[y]){
                    instance->registers[x] = instance->registers[x] - instance->registers[y];
                    instance->registers[vF] = 0;
                }
                else{
                    instance->registers[vF] = 1;
                }
                break;
            case 0x6:
                if(instance->registers[x]%2==0){
                    instance->registers[x] = instance->registers[x]>>1;
                    instance->registers[vF] = 0;
                }
                else{
                    instance->registers[x] = instance->registers[x]>>1;
                    instance->registers[vF] = 1;
                }
                break;
            case 0x7:
                    if(instance->registers[y]>instance->registers[x]){
                        instance->registers[vF] = 1;
                    }
                    else{
                        instance->registers[x]-=instance->registers[y];
                    }
                break;
            case 0xE:
                if(instance->registers[x]%2==0){
                    instance->registers[x] = instance->registers[x]<<1;
                    instance->registers[vF] = 0;
                }
                else{
                    instance->registers[x] = instance->registers[x]<<1;
                    instance->registers[vF] = 1;
                }
            break;
        break;
    }
    case 0x9000:
        if(instance->registers[x]!=instance->registers[y]){
            instance->program_counter += 2;
        }
    break;  
    case 0xA000:
        instance->I = n3;
    break;
    case 0xB000:
        instance->program_counter = n3 + instance->registers[0];
    break;
    case 0xC000:
        instance->registers[x] = (rand()%256) & n2;
    break;
    case 0xD000:

    break;
    
    case 0xE000:
        switch(opcode & 0xFF){
            case 0x9E:
                if(movement[x])
                    instance->program_counter+=2;
            break;
            case 0xA1:
                if(!movement[x])
                    instance->program_counter+=2;
            break;
        break;
        }
        
    case 0xF000:
        switch(opcode & 0xFF){
            case 0x07:
                instance->registers[x] = instance->delay_timer;
            break;
            case 0x0A:
                // wait for a keypress
            break;
            case 0x15:
                instance->delay_timer = instance->registers[x];
            break;
            case 0x18:
                instance->sound_timer = instance->registers[x];
            break;
            case 0x1E:
                instance->I += instance->registers[x];
            break;
            case 0x29:
                // set I to the location for the hexadeciaml spride of value vx
            break;
            case 0x33:
                // what the fuck
            break;
            case 0x55:
                // what
                // for(int i = instance->I; i<=instance->I+)
            break;
            case 0x65:
                // what
            break;
        break;
        }
    break;
        

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