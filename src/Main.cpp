//TODO: сделать свою векторную матешу
// TODO: скрипт двигло
// TODO: сделать кластерный рендер
//TODO: начать делать хотя бы что-то со звуком 
//TODO: реализовать систему частиц(партикле сустемXD) 
//TODO: привести все фабричные методы к одному виду
//TODO: Уменьшить кол-во дескрипторов.
//TODO: заменить все shared_ptr на unique потому что так шевцов сказал?
/*
TODO: подумать над реализацей:
****************************************************************************************
 * воркспейса и вообще, хорошая ли это затея разделять эту сущность на редактор и логику
 * реализовать каким-то хуем папочку в ведроиде, откуда данные таскать буду
 * передачи данных через потоки.(думаеца создать очередь тасков для каждой подсистемы движка)
****************************************************************************************

*/

#include "stdafx.h"
#include "Engine.hpp"


#ifndef __ANDROID__
#undef main
#endif
#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char* argv[])
{
    BEbraEngine::Engine engine;
    engine.Main();

    return 1337228;
}
