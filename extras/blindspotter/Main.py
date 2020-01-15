'''
Created on Jan 6, 2020

@author: NOOK
'''

from tkinter import *
from math import pi
from numpy import linspace, sin, cos
import serial;

BLACK = "black"
WHITE = "white"
RED = 'red'
YELLOW = 'yellow'
BLUE = 'BLUE'



class GFX:
    def __init__(self, canvas):
        self.canvas = canvas;

    def drawPixel( self, x : int, y : int, color : int ):
        pass
    
    def drawLine( self, x0 : int, y0 : int, x1 : int, y1 : int, color : int):
        self.canvas.create_line( x0, y0, x1, y1, fill=color)
    
    def drawRect( self, x0 : int, y0 : int, w : int, h : int, color : int):
        self.canvas.create_rectangle(x0, y0, x0+w, y0+h, outline=color)
    
    def fillRect( self, x0 : int, y0 : int, w : int, h : int, color : int):
        pass
    
    def drawCircle( self, x : int, y : int, r : int, color : int):
        self.canvas.create_oval(x-r, y-r, x+r, y+r, outline=color)
    
    def fillCircle( self, x : int, y : int, r : int, color : int):
        self.canvas.create_oval(x-r, y-r, x+r, y+r, outline=color, fill=color)
    
    def drawRoundRect( self, x0 : int, y0 : int, w : int, h : int, r : int, color : int):
        pass
    
    def fillRoundRect( self, x0 : int, y0 : int, w : int, h : int, r : int, color : int):
        pass

    def drawTriangle( self, x0 : int, y0 : int, x1 : int, y1 : int, x2 : int, y2 : int, color : int):
        self.canvas.create_polygon( x0, y0, x1, y1, x2, y2, outline=color, fill='')
    
    def fillTriangle( self, x0 : int, y0 : int, w : int, h : int, x2 : int, y2 : int, color : int):
        self.canvas.create_rectangle(x0, y0, x0+w, y0+h, fill=color)
    

class UiConfig:
    def __init__(self):
        self.screenWidth = 320
        self.screenHeight = 240

def configure() :
    coachLength = 380 # in
    coachWidth  = 100 # in
    hitchLength = 10 # inches; 0 means no trailer
    trailerLength = 50 # in
    trailerWidth = 90  # in
    leftSensors = [85, 220, 360]
    rightSensors = [85, 220, 360]
    markerBox = 5
    laneWidth = 144 # in 12 feet
    laneDash = 10
    beamwidth = 70;

    ui = UiConfig()
    
    ui.centerX = ui.screenWidth // 2
    ui.centerY = ui.screenHeight // 2;
    ui.margin = 5 # pixels
    overallLength = coachLength + hitchLength + trailerLength;
    imageHeight = ui.screenHeight - 2*ui.margin;
    scale = imageHeight / overallLength;
    ui.scale = scale;
    ui.beamN = 5;
    ui.beamCos = cos(pi/180 * linspace(-beamwidth/2,beamwidth/2,7));
    ui.beamSin = sin(pi/180 * linspace(-beamwidth/2,beamwidth/2,7));
    
    ui.coach = [ui.centerX - (int) (scale * 0.5 * coachWidth),
                ui.centerY - (int) (scale * 0.5 * overallLength),
                (int) (scale * coachWidth), (int) (scale * coachLength)];
    ui.hasTrailer = True;
    ui.trailer = [ui.centerX - (int) (scale * 0.5 * trailerWidth),
                  ui.coach[1] + (int) (scale * (coachLength + hitchLength)),
                  (int) (scale * trailerWidth), (int) (scale * trailerLength)];
    ui.left = [
                [ui.coach[0], ui.coach[1] + (int)(scale*leftSensors[0]), markerBox],
                [ui.coach[0], ui.coach[1] + (int)(scale*leftSensors[1]), markerBox],
                [ui.coach[0], ui.coach[1] + (int)(scale*leftSensors[2]), markerBox],
              ]
    ui.right = [
                [ui.coach[0]+ui.coach[2], ui.coach[1] + (int)(scale*rightSensors[0]), markerBox],
                [ui.coach[0]+ui.coach[2], ui.coach[1] + (int)(scale*rightSensors[1]), markerBox],
                [ui.coach[0]+ui.coach[2], ui.coach[1] + (int)(scale*rightSensors[2]), markerBox],
              ]
    ui.laneDash = 10 # pixels
    ui.laneX = [ui.centerX - (int)(scale*1.5*laneWidth),
                ui.centerX - (int)(scale*0.5*laneWidth),
                ui.centerX + (int)(scale*0.5*laneWidth),
                ui.centerX + (int)(scale*1.5*laneWidth)]
                  
    attrs = vars(ui)
    print(attrs)
    return ui

def drawDetector(gfx, detector, distance):
    if (distance > 9000) :
        gfx.fillCircle(detector[0], detector[1], detector[2], BLUE)
    elif (distance > 0) :
        gfx.fillCircle(detector[0], detector[1], detector[2], BLACK)
    elif (distance == 0) :
        gfx.fillCircle(detector[0], detector[1], detector[2], WHITE)
    elif (distance == -1) :
        gfx.fillCircle(detector[0], detector[1], detector[2], RED)
    else:
        gfx.fillCircle(detector[0], detector[1], detector[2], YELLOW)
    
def drawDetection(gfx, config, side, detector, range ):
    gfx.drawTriangle(detector[0], detector[1], 
                     detector[0]+(int)(side * config.scale * range * config.beamCos[0]), 
                     detector[1]-(int)(config.scale * range * config.beamSin[0]),
                     detector[0]+(int)(side * config.scale * range * config.beamCos[0]),
                     detector[1]+(int)(config.scale * range * config.beamSin[0]), RED)
    
def drawDetection2(gfx, config, side, detector, distance ):
    if (distance > 0 and distance < 9000) :  # mm
        distance *= 12*1e-3/0.3048
        for i in range(0,len(config.beamCos)):
            gfx.drawLine(detector[0], detector[1], 
                         detector[0]+(int)(side * config.scale * distance * config.beamCos[i]),
                         detector[1]-(int)(config.scale * distance * config.beamSin[i]), RED)
        for i in range(1,len(config.beamCos)):
            gfx.drawLine(detector[0]+(int)(side * config.scale * distance * config.beamCos[i-1]),
                         detector[1]-(int)(config.scale * distance * config.beamSin[i-1]),
                         detector[0]+(int)(side * config.scale * distance * config.beamCos[i]),
                         detector[1]-(int)(config.scale * distance * config.beamSin[i]),  
                         RED)
        
def drawBackground(gfx, config):
    gfx.canvas.delete("all")
    gfx.drawRect( config.coach[0], config.coach[1], config.coach[2], config.coach[3], BLACK )
    if (config.hasTrailer) :
        gfx.drawRect( config.trailer[0], config.trailer[1], config.trailer[2], config.trailer[3], BLACK )
    for i in range(0, len(config.left)):
        drawDetector(gfx, config.left[i], False);
    for i in range(0, len(config.right)):
        drawDetector(gfx, config.right[i], False);
    for x in config.laneX :
        for y in range(0, config.screenHeight, 2*config.laneDash) :
            gfx.drawLine(x, y, x, y+config.laneDash, BLACK)
    


def timerEventOn(gfx, config) :
    drawBackground(gfx, config)
    for i in range(0,6) :
        gfx.distanceVars[i].set( str(gfx.distance[i]) )
    drawDetector(gfx, config.left[0], gfx.distance[0]);
    drawDetection2(gfx, config, -1.0, config.left[0], gfx.distance[0] );
    drawDetector(gfx, config.left[1], gfx.distance[1]);
    drawDetection2(gfx, config, -1.0, config.left[1], gfx.distance[1] );
    drawDetector(gfx, config.left[2], gfx.distance[1]);
    drawDetection2(gfx, config, -1.0, config.left[2], gfx.distance[2] );

    drawDetector(gfx, config.right[0], gfx.distance[3]);
    drawDetection2(gfx, config, +1.0, config.right[0], gfx.distance[3] );
    drawDetector(gfx, config.right[1], gfx.distance[4]);
    drawDetection2(gfx, config, +1.0, config.right[1], gfx.distance[4] );
    drawDetector(gfx, config.right[2], gfx.distance[5]);
    drawDetection2(gfx, config, +1.0, config.right[2], gfx.distance[5] );

    gfx.canvas.after(1000, timerEventOn, gfx, config)
    
# def timerEventOff(gfx, config) :
#     drawBackground(gfx, config)
#     drawDetector(gfx, config.right[0], True);
#     drawDetection2(gfx, config, +1.0, config.right[0], gfx.distance[3] );
#     drawDetector(gfx, config.right[1], True);
#     drawDetection2(gfx, config, +1.0, config.right[1], gfx.distance[4] );
#     drawDetector(gfx, config.right[2], True);
#     drawDetection2(gfx, config, +1.0, config.right[2], gfx.distance[5] );
#     gfx.canvas.after(1000, timerEventOn, gfx, config)
    
class Link :
    def __init__(self, serial):
        self.serial = serial;
        self.buffer = '';
        
    def cksum(self, buffer):
        cs = 0
        for b in buffer:
            cs = cs ^ b
        return hex(cs)[2:].zfill(2).upper()
        
    def decode(self, gfx, line):
        major = line[1:].split(';')
        if (major[8] == self.cksum(line[0:len(line)-2].encode('UTF-8'))) :
            fields = major[0].split(',')
            gfx.clockVar.set( int(fields[0]) )
            n = int(fields[1])
            for i in range(0,n):
                fields = major[1+i].split(',')
                if major[1+i].endswith('0,0,0') :
                    gfx.distance[i] = int(fields[0])
                elif (fields[2] != '0') :
                    gfx.distance[i] = -1;
                elif (fields[3] != '0') :
                    gfx.distance[i] = -2;
                else :
                    gfx.distance[i] = 0;
            print(major[0], gfx.distance )
            
def serviceSerial( gfx, link ):
    if (link.serial.is_open) :
        if (link.serial.in_waiting != 0) :
            ch = link.serial.read(size=link.serial.in_waiting)
            link.buffer += ch.decode('UTF-8')
            if (link.buffer.endswith('\n')) :
                line = link.buffer;
                n = len(line);
#                 print(line)
                if (n > 3 and line[0] == '$' and line[n-2] == '$') :
                    link.decode(gfx, line[0:n-2])
                else :
                    print("IGNORED: ", line)
                link.buffer = '';
            
        gfx.canvas.after(1, serviceSerial, gfx, link );
    
def main(link):
    config = configure();
    
    master = Tk()
    
    w = Canvas( width=config.screenWidth, height=config.screenHeight )
    w.grid(row=1, column=0)
    
    gfx = GFX(w);
    gfx.clockVar = StringVar();
    gfx.distance = [120, 0, 0, 0, 0, 0];
    gfx.distanceVars = [
        StringVar(), StringVar(), StringVar(),
        StringVar(), StringVar(), StringVar(),
        ];
    gfx.labels = [
        Label( master, textvariable=gfx.clockVar ).grid(row=0, column=0),

        Label( master, textvariable=gfx.distanceVars[0]).grid(row=2, column=0),
        Label( master, textvariable=gfx.distanceVars[1]).grid(row=3, column=0),
        Label( master, textvariable=gfx.distanceVars[2]).grid(row=4, column=0),
        Label( master, textvariable=gfx.distanceVars[3]).grid(row=2, column=1),
        Label( master, textvariable=gfx.distanceVars[4]).grid(row=3, column=1),
        Label( master, textvariable=gfx.distanceVars[5]).grid(row=4, column=1),
        ];
    gfx.clockVar.set('Clock')
    gfx.distanceVars[0].set('Left0')
    gfx.distanceVars[1].set('Left1')
    gfx.distanceVars[2].set('Left2')
    gfx.distanceVars[3].set('Right0')
    gfx.distanceVars[4].set('Right1')
    gfx.distanceVars[5].set('Right2')

    w.after(1000, timerEventOn, gfx, config)
    w.after(1, serviceSerial, gfx, link );
    mainloop()
    
if __name__ == '__main__':
    with serial.Serial('COM7:', 115200, timeout=1) as ser:
        link = Link(ser);
        main(link)
#         while( ser.is_open) :
#             try :
#                 line = ser.readline().decode('UTF-8');
#                 n = len(line);
#                 if (n > 3 and line[0] == '$' and line[n-2] == '$') :
#                     print(line[1:n-2].split(';'))
#                 else :
#                     print("ERROR:", line)
#             except :
#                 pass