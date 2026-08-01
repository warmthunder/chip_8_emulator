#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
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


// ch8 components
typedef struct{
    uint8_t registers[16];
    uint8_t memory[4096];
    uint16_t program_counter;
    stackk stack;
    uint8_t stack_pointer;
    uint8_t delay_timer;
    uint8_t sound_timer;
    // bool keypress[16];
    uint32_t display[64*32];
    uint16_t I;
}chip8;

void stk_push(stackk* stack, uint16_t value, chip8* instance){
    if(stack->top >= stack_length-1){
        printf("%s","Stack is full");
        return;
    }
    stack->stk[stack->top] = value;
    stack->top ++;
    instance->stack_pointer = stack->top;
}

uint16_t stk_pop(stackk* stack, chip8 *instance){
    if(stack->top == 0)
        return -1;
    stack->top--;
    instance->stack_pointer = stack->top;
    return stack->stk[stack->top];
}

// SDL
SDL_Window *window;
SDL_Renderer *renderer;
bool quit = false;
bool keys[16];
int keypress = 0;
const uint8_t fontset[80] = {
    // 0
    0xF0, 0x90, 0x90, 0x90, 0xF0,

    // 1
    0x20, 0x60, 0x20, 0x20, 0x70,

    // 2
    0xF0, 0x10, 0xF0, 0x80, 0xF0,

    // 3
    0xF0, 0x10, 0xF0, 0x10, 0xF0,

    // 4
    0x90, 0x90, 0xF0, 0x10, 0x10,

    // 5
    0xF0, 0x80, 0xF0, 0x10, 0xF0,

    // 6
    0xF0, 0x80, 0xF0, 0x90, 0xF0,

    // 7
    0xF0, 0x10, 0x20, 0x40, 0x40,

    // 8
    0xF0, 0x90, 0xF0, 0x90, 0xF0,

    // 9
    0xF0, 0x90, 0xF0, 0x10, 0xF0,

    // A
    0xF0, 0x90, 0xF0, 0x90, 0x90,

    // B
    0xE0, 0x90, 0xE0, 0x90, 0xE0,

    // C
    0xF0, 0x80, 0x80, 0x80, 0xF0,

    // D
    0xE0, 0x90, 0x90, 0x90, 0xE0,

    // E
    0xF0, 0x80, 0xF0, 0x80, 0xF0,

    // F
    0xF0, 0x80, 0xF0, 0x80, 0x80
};

static void sdl_process_event(SDL_Event *event){
   switch(event->type){
        case SDL_KEYDOWN:
            SDL_Keycode key = event->key.keysym.sym;
            switch(key){
                case SDLK_s:
                    keys[0] = true;
                    keypress = 1;
                    break;
                case SDLK_w:
                    keys[1] = true;
                    keypress = 2;
                    break;
                case SDLK_a:
                    keys[2] = true;
                    keypress = 3;
                    break;
                case SDLK_d:
                    keys[3] = true;
                    keypress = 4;
                    break;
            break;
            }
            // if(key==SDLK_s)
            //     movement[1] = +10;
            // if(key==SDLK_w)
            //     movement[1] = -10;
            // break;
         case SDL_KEYUP:
                key = event->key.keysym.sym;
               switch(key){
                case SDLK_s:
                    keys[0] = false;
                    keypress = 0;
                    break;
                case SDLK_w:
                    keys[1] = false;
                    keypress = 0;
                    break;
                case SDLK_a:
                    keys[2] = false;
                    keypress = 0;
                    break;
                case SDLK_d:
                    keys[3] = false;
                    keypress = 0;
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
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    SDL_UpdateTexture(texture, NULL, instance->display, 64*sizeof(uint32_t));
    SDL_RenderCopy(renderer,texture,NULL,NULL);
    SDL_RenderPresent(renderer);
    return 0;
    }

// void fetch(){

// }

void decode(uint32_t opcode, chip8* instance){
uint8_t x = (opcode & 0x0F00) >> 8;
uint8_t y = (opcode & 0x00F0) >> 4;
uint16_t sum = instance->registers[x] + instance->registers[y];
uint8_t vF = 0xF;
uint16_t n3 = opcode & 0x0FFF;
uint16_t n2 = opcode & 0x00FF;

switch(opcode){
    case 0x00E0:
        memset(instance->display, 0, sizeof(instance->display)); 
        break;
    case 0x00EE:
        instance->program_counter = stk_pop(&instance->stack, instance);
        // instance->stack_pointer--;
        break;
    break;
}
switch(opcode & 0xF000){
    case 0x1000:
        instance->program_counter = opcode & 0x0FFF;
        break;
    case 0x2000:
        stk_push(&instance->stack, instance->program_counter, instance);
        instance->program_counter = opcode & 0x0FFF;
        break;
    case 0x3000:
        if(instance->registers[x] == (opcode&0xFF))
        // skip next instruction if condition is met
            instance->program_counter += 2;
        break;
    case 0x4000:
        if(instance->registers[x] != (opcode&0xFF))
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
                instance->registers[x] = instance->registers[x] - instance->registers[y];
                if(instance->registers[x]> instance->registers[y]){
                    
                    instance->registers[vF] = 1;
                }
                else{
                    instance->registers[vF] = 0;
                }
                break;
            case 0x6:
                instance->registers[x] = instance->registers[x]>>1;
                if(instance->registers[x]%2==0){ 
                    instance->registers[vF] = 0;
                }
                else{
                    instance->registers[vF] = 1;
                }
                break;
            case 0x7:
                instance->registers[x] = instance->registers[y] - instance->registers[x];
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
        instance->I = opcode&0x0FFF;
    break;
    case 0xB000:
        instance->program_counter = (opcode&0x0FFF) + instance->registers[0];
    break;
    case 0xC000:
        instance->registers[x] = (rand()%256) & n2;
    break;
    case 0xD000:
        uint8_t sprite[15*8] = {0};
        instance->registers[vF] = 0;
        for(int i = 0; i<(opcode&0xF);i++){
            for(int j = 0;j<8;j++){
            sprite[i*8 + j] = ((instance->memory[instance->I+i])>>(7-j))&1; 
            if(instance->display[((instance->registers[y] + i)%32) * 64 + ((instance->registers[x] + j)%64)] == 1 && sprite[i*8 + j] ==1 ){
                instance->registers[vF] = 1;
            }
            instance->display[((instance->registers[y] + i)%32) * 64 + ((instance->registers[x] + j)%64)] ^= sprite[i*8 + j];
            if (instance->display[((instance->registers[y] + i)%32) * 64 + ((instance->registers[x] + j)%64)])
                instance->display[((instance->registers[y] + i)%32) * 64 + ((instance->registers[x] + j)%64)] = 0xFFFFFFFF;
        }
        }
        
        break;
    
    case 0xE000:
        switch(opcode & 0xFF){
            case 0x9E:
                if(keys[instance->registers[x+1]])
                    instance->program_counter+=2;
            break;
            case 0xA1:
                if(!keys[instance->registers[x+1]])
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
                // fix this, this does not work
                while(keypress == 0){
                    // while( SDL_PollEvent(&event)){
                    // sdl_process_event(&event);
                    // }
                    SDL_Delay(10);
                }
                instance->registers[x] = keypress;

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
                instance->I = x*5;
            break;
            case 0x33:
                instance->memory[instance->I] = (instance->registers[x] & 0b0100);
                instance->memory[instance->I+1] = (instance->registers[x] & 0b0010)>>1;
                instance->memory[instance->I+2] = (instance->registers[x] & 0b0001)>>2;
            break;
            case 0x55:
                // what
                for(int i = 0; i<=x;i++)
                    instance->memory[instance->I+i] = instance->registers[i];  
            break;
            case 0x65:
                for(int i = 0; i<=x;i++)
                    instance->registers[i] = instance->memory[instance->I+i];
            break;
        break;
        }
    break;
        

break;

}

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
    size_t bytes = loadromtomem(&instance, "IBM.ch8");
    memset(instance.display, 0, sizeof(instance.display)); 
    // adding digit spirtes to memory
    for(int i = 0; i<80;i++){
        instance.memory[i] = fontset[i];
    }

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

    // init chip8
    instance.program_counter = 0x200;

    SDL_Event event;
    while(!quit){

        uint16_t opcode = instance.memory[instance.program_counter]<<8 | instance.memory[instance.program_counter+1];
        instance.program_counter+= 2;
        printf("%04X\n", opcode);
        fflush(stdout);
        decode(opcode, &instance);
        // printf("%d",instance.program_counter);
        // input
        while( SDL_PollEvent(&event)){
            sdl_process_event(&event);
        }
        render(renderer, screen, &instance);
    }

    return 0;
}