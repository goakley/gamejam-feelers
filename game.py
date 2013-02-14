#!/usr/bin/python

import threading, time, random, math
import pygame
from pygame.locals import *
pygame.init()


###
### --- DATA ---
###

# PLAYER
class Player :
    def __init__(self) :
        self._heart = Player.Heart()
        self._heart.start()
        self.pos_x = self.pos_y = 0.0
        self.rotation = random.random()*360
    def __del__(self) :
        self._heart.stop()
    # PLAYER.HEART
    class Heart(threading.Thread) :
        _channel = pygame.mixer.Channel(0)
        _sound = pygame.mixer.Sound("audio/heartbeat.wav")
        def __init__(self) :
            super(Player.Heart, self).__init__()
            self._running = False
            self.bpm = 60
        def run(self) :
            while self._running :
                Player.Heart._channel.play(Player.Heart._sound)
                time.sleep(60.0/self.bpm)
        def start(self) :
            self._running = True
            super(Player.Heart, self).start()
        def stop(self) :
            self._running = False

# FINISH
class Finish :
    def __init__(self, x, y) :
        self._channel = pygame.mixer.Channel(1)
        self._sound = pygame.mixer.Sound("audio/bell.wav")
        self._channel.play(self._sound, -1)
        self.pos_x = x
        self.pos_y = y

# CREATURE
class Creature :
    def __init__(self, x, y) :
        self._channel=pygame.mixer.Channel(Creature.Audio._channels_free.pop(0))
        self._sound = Creature.Audio.choose_sound()
        self._channel.play(self._sound, -1)
        self.pos_x = x
        self.pos_y = y
    # CREATURE.AUDIO
    class Audio(threading.Thread) :
        _channels_free = range(2,pygame.mixer.get_num_channels())
        _files = ["growl.wav"]
        @staticmethod
        def choose_sound() :
            return pygame.mixer.Sound("audio/" + random.choice(["growl.wav"]))


def calculate_balance(listener_pos, listener_angle, emitter_pos) :
    dist_x = emitter_pos[0] - listener_pos[0]
    dist_y = emitter_pos[1] - listener_pos[1]
    distance = math.sqrt(dist_x*dist_x + dist_y*dist_y)
    if (distance == 0.0) :
        return (1.0,1.0,1.0)
    volume = (128.0-distance)/128.0
    if (volume <= 0) :
        return (0.0,1.0,1.0)
    angle = math.acos(dist_x/distance)*(-1 if (dist_y<0) else 1)-listener_angle
    norm_left = (math.sin(angle)+1.0)/3.0 + (1.0/3.0)
    norm_right = (-math.sin(angle)+1.0)/3.0 + (1.0/3.0)
    return (volume, norm_left, norm_right)


###
### --- GAME ---
###


###
# GAME - INIT
display = pygame.display.set_mode((640,480), pygame.RESIZABLE)#pygame.display.list_modes()[0], pygame.FULLSCREEN)
pygame.key.set_repeat(1)
pygame.mouse.set_visible(False)
font = pygame.font.SysFont(pygame.font.get_default_font(), 32)
clock = pygame.time.Clock()
player = Player()
finish = Finish(0,50)
creatures = [Creature(50,50)]


###
# GAME - LOOP
rungame = True
lastloop = pygame.time.get_ticks()
while rungame :
    # INPUT
    presses = pygame.key.get_pressed()
    if presses[K_LEFT] or presses[K_a] :
        player.rotation = (player.rotation-2)%360
    if presses[K_RIGHT] or presses[K_d] :
        player.rotation = (player.rotation+2)%360
    if presses[K_UP] or presses[K_w] :
        rotation = math.radians(player.rotation)
        player.pos_x += math.cos(rotation)
        player.pos_y += math.sin(rotation)
    for event in pygame.event.get() :
        if event.type == QUIT :
            rungame = False
        elif event.type == KEYDOWN :
            if event.key == K_ESCAPE :
                pygame.event.post(pygame.event.Event(QUIT))
        #elif event.type == MOUSEMOTION :
        #    player.rotation = (player.rotation+event.rel[0]/10.0)%360
    # UPDATE
    vol,l,r = calculate_balance((player.pos_x,player.pos_y), 
                                math.radians(player.rotation), 
                                (finish.pos_x,finish.pos_y))
    finish._sound.set_volume(vol)
    finish._channel.set_volume(l,r)
    for creature in creatures :
        vol,l,r = calculate_balance((player.pos_x,player.pos_y), 
                                    math.radians(player.rotation), 
                                    (creature.pos_x,creature.pos_y))
        creature._sound.set_volume(vol)
        creature._channel.set_volume(l,r)
    # DISPLAY
    display.fill(pygame.Color(0,0,0))
    #text = font.render(`player.rotation`, False, pygame.Color(255,255,255))
    #display.blit(text, text.get_rect())
    #text = font.render(`(player.pos_x, player.pos_y)`, False, pygame.Color(255,255,255))
    #text_r = text.get_rect()
    #text_r.topleft = (0,50)
    #display.blit(text, text_r)
    pygame.display.update()
    clock.tick(30)


###
# GAME - QUIT
del player
pygame.quit()
