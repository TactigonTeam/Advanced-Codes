from bluepy.btle import Scanner, DefaultDelegate, Peripheral, AssignedNumbers, BTLEException
import threading, binascii, sys, json, logging
from ConfigParser import SafeConfigParser
from struct import *
import pygame
import time
import numpy as np
import quaternion
from pyquaternion import Quaternion
import math
import numpy




logging.basicConfig(level=logging.DEBUG, format='[%(levelname)s] (%(threadName)-10s) %(message)s')

def log_it(*args):
    msg = " ".join([str(a) for a in args])
    logging.debug(msg)



#Graphic device:
#
# collect info used to draw a device on screen
#
class TactiDevice():

    def __init__(self, mac, tactiID):
        self.mac = mac
        self.tactiID = tactiID
        self.roll = 0.;
        self.pitch = 0.;
        self.yaw = 0.;
        self.q0 = 0;
        self.q1 = 0;
        self.q2 = 0;
        self.q3 = 0;
        self.currAngle = 0;
        #self.zeroQuat = quaternion.quaternion(1, 0, 0, 0)
        self.zeroQuat = Quaternion(w=1, x=0, y=0, z=0)
        self.oneDataReceived = False
        self.xAtThisIdx = -1
        self.rotSign = 1
        
    

#
# delegate used to process data coming from BLE characteristic
#
class MyDelegate(DefaultDelegate):

    def __init__(self, addr, tactiID):
        DefaultDelegate.__init__(self)
        self.id = addr
        self.tactiID = tactiID

    # Called by BluePy when an event was received.
    def handleNotification(self, cHandle, data):
        
        #expected data: q0(4) | q1(4) | q2(4) | q3(4)
        
        
        self.d = data
        
        #get q0,q1,q2,q3 from characteristic
        gFoundDevices[self.tactiID].q0 = unpack('f', data[0:4])[0]               
        gFoundDevices[self.tactiID].q1 = unpack('f', data[4:8])[0]
        gFoundDevices[self.tactiID].q2 = unpack('f', data[8:12])[0]
        gFoundDevices[self.tactiID].q3 = unpack('f', data[12:16])[0]
        
                    
        #logging.info("Received notification from: %s. quat: %f %f %f %f", self.id, gFoundDevices[self.tactiID].q0, gFoundDevices[self.tactiID].q1, gFoundDevices[self.tactiID].q2, gFoundDevices[self.tactiID].q3)
        
        
        #mark that at lest one packed has been received (needed for zero)
        dev = findDev(self.tactiID)
        dev.oneDataReceived = True;
        

        

#
# GUI Thread
#
class Graphichread(threading.Thread):
    ## @var WAIT_TIME
    # Time of waiting for notifications in seconds
    WAIT_TIME = 0.1
    

    def __init__(self):
        threading.Thread.__init__(self)
                

    def run(self):
        while True:
            #draw graphics
            tacti_graphics() 




#
# Thread handling a single Tactigon board
#
class BleThread(Peripheral, threading.Thread):
    ## @var WAIT_TIME
    # Time of waiting for notifications in seconds
    WAIT_TIME = 0.1
    ## @var EXCEPTION_WAIT_TIME
    # Time of waiting after an exception has been raiesed or connection lost
    EXCEPTION_WAIT_TIME = 10
    # We'll write to this
    txUUID = "bea5760d-503d-4920-b000-101e7306b005"

    def __init__(self, peripheral_addr, tactiID):
        try:
            Peripheral.__init__(self, peripheral_addr)
        except Exception, e:
            logging.warning("Problem initializing Peripheral %s", e.message)
            raise
            

        threading.Thread.__init__(self)        
        self.tactiID = tactiID
        
        # Set up our WRITE characteristic
        try:
            self.txh = self.getCharacteristics(uuid=self.txUUID)[0]
        except Exception, e:
            logging.debug("Problem getting characteristics from %s. %s", self.addr, e.message)
        
        # Create the BluePy objects for this node
        self.delegate = MyDelegate(peripheral_addr, self.tactiID)
        self.withDelegate(self.delegate)
        self.connected = True
        

        logging.info("Configuring RX to notify me on change")
        try:
            # notify us on characteristic change            
            handle = self.txh.getHandle()            
            self.writeCharacteristic(handle+1, b"\x01\x00", withResponse=True)
        except Exception, e:
            logging.debug("Problem subscribing to RX notifications: %s", e.message)


    def run(self):
        while self.connected:
            try:
                self.waitForNotifications(self.WAIT_TIME)
                                                                                                       
            except BaseException, be:
                logging.debug("BaseException caught: %s", be.message)
                self.connected = False
                
            except BTLEException, te:
                logging.debug("BTLEException caught for %s. %s", self.addr, te.message)
                if str(te.message) == 'Device disconnected':
                    logging.debug("Device disconnected: %s", self.addr)
                    self.connected = False
                    # We don't want to call waitForNotifications and fail too often
                    time.sleep(self.EXCEPTION_WAIT_TIME)
                else:
                    raise
                    
            except Exception, e:
                logging.debug("Peripheral exception for %s. %s", self.addr, e.message)
                self.connected = False
                



#search device in gFoundDevices
def findDev(targetID):
    for dID in gFoundDevices:
        dev = gFoundDevices[dID]
        if(dev.tactiID == targetID):
            return dev

    #if I get here means that I didn't found any dev. Return an empty one
    return TactiDevice("", targetID)
    


#print a text message on screen 
def messageOnSceen(text, x, y):

    myfont = pygame.font.SysFont('Comic Sans MS', 40)
    textsurface = myfont.render(str(text), True, (255, 0, 0))
    screen.blit(textsurface,(x,y))    
    
    


def rotateXvect(dev):
    
    #get current quat for given device
    qNow = Quaternion(w=dev.q0, x=dev.q1, y=dev.q2, z=dev.q3).normalised
    q_rot = dev.zeroQuat * qNow.conjugate                    
    
    #rotate z vector                          
    ref_axis = numpy.array([1., 0., 0.])      
    xVect = q_rot.rotate(ref_axis)
    
    return xVect    


def rotateYvect(dev):
    
    #get current quat for given device
    qNow = Quaternion(w=dev.q0, x=dev.q1, y=dev.q2, z=dev.q3).normalised
    q_rot = dev.zeroQuat * qNow.conjugate                    
    
    #rotate z vector                          
    ref_axis = numpy.array([0., 1., 0.])      
    yVect = q_rot.rotate(ref_axis)
    
    return yVect    
    

def rotateZvect(dev):
    
    #get current quat for given device
    qNow = Quaternion(w=dev.q0, x=dev.q1, y=dev.q2, z=dev.q3).normalised
    q_rot = dev.zeroQuat * qNow.conjugate                    
    
    #rotate z vector                          
    ref_axis = numpy.array([0., 0., 1.])      
    zVect = q_rot.rotate(ref_axis)
    
    return zVect



# handle graphics
def tacti_graphics():
    
    global gMilliTicks
    global gZeroMilliTicks
    global gZeroDone
    global gGlobalYaw_2
    global gGlobalYaw_3
    
    zeroPoint = (gWINDOW_SIZE[0]/2, gWINDOW_SIZE[1]/2)    
    rectSize = (gRECT_HEIGHT, gRECT_WIDTH)
    
    #update graphics every 20msec
    if (time.time()*1000 > (gMilliTicks + 20)):
        gMilliTicks = time.time()*1000
        
        
        #Clear the screen and set the screen background
        screen.fill(GRAY)
        logo_top = screen.get_height() - gLogoImge.get_height()
        logo_left = screen.get_width()/2 - gLogoImge.get_width()/2
        screen.blit(gLogoImge, (logo_left, logo_top))
        
        
        #compute initial zero
        if(gZeroDone == 0):         #STEP 0: get zero quaternion
            
            #check if I have received at least lest one packedt for every device
            received = 0
            for dID in gFoundDevices:                   
                dev = gFoundDevices[dID]
                if(dev.oneDataReceived == True):
                    received += 1
            
            #if yes do zero        
            if(received == len(gExpectedDevices)):       
             
                for dID in gFoundDevices:                
                    dev = gFoundDevices[dID]
                    dev.zeroQuat = Quaternion(w=dev.q0, x=dev.q1, y=dev.q2, z=dev.q3).normalised                    
                    logging.info("calibration done for %s", dID)
                    logging.info("axis: %f %f %f, angle: %f", dev.zeroQuat.axis[0], dev.zeroQuat.axis[1], dev.zeroQuat.axis[2], dev.zeroQuat.angle)
                                                            
                gZeroDone = 1
                                               
            else:                
                messageOnSceen("Waiting for devices", 30, 30)
        
        elif(gZeroDone == 1):       #STEP 1: check XY axis
                        
            messageOnSceen("Devices found", 30, 30)
            messageOnSceen("Calibration STEP 1 done", 30, 60)
            messageOnSceen("Please put chain in horiz position", 30, 90)
             
            #wait vertical position for every device 
            for dID in gFoundDevices:                   
                dev = gFoundDevices[dID]
                
                if(dev.xAtThisIdx < 0):
                    zVectRotated = rotateZvect(dev)
                
                    x = zVectRotated[0]
                    y = zVectRotated[1]
                    z = zVectRotated[2]
                    if(abs(x) > 0.7):
                        dev.xAtThisIdx = 0    
                        dev.rotSign = -1 * math.copysign(1,x)                    
                        logging.info("dev %s has x in 0 (%f)", dID, x)
                    elif(abs(y) > 0.7):
                        dev.xAtThisIdx = 1
                        dev.rotSign = -1 * math.copysign(1,y)                         
                        logging.info("dev %s has x in 1 (%f)", dID, y)
                    else:
                        dev.xAtThisIdx = -1
                        #if(dID == '1'):    
                        #    logging.info("axis %f %f %f", x, y, z)
            
            #check if step 2 has been completed for every device
            step1Done = 0 
            for dID in gFoundDevices:                   
                dev = gFoundDevices[dID]
                if(dev.xAtThisIdx >= 0):
                    step1Done += 1
                    
            if(step1Done == 3):         #len(gExpectedDevices)):
                gZeroDone = 2
                logging.info("calibration Step 2 done, go to running")
                                                    
                                                        
        else:                       #STEP 2: running               
             
            #compute angle for every connected devices
            for dID in gFoundDevices:
                   
                dev = gFoundDevices[dID]  
                                                            
                #compute pitch from quaternions rotating a Z vector                                                          
                zVect = rotateZvect(dev)
                
                if(dev.xAtThisIdx == 0):
                    x = zVect[0] * dev.rotSign
                    y = zVect[1]
                    z = zVect[2]
                else:
                    x = zVect[1] * dev.rotSign
                    y = zVect[0]
                    z = zVect[2]
                
                #compute angle and store in dev                
                a1 = x
                a2 = z / (math.sqrt(1 - pow(y,2)))
                angle = -math.degrees(math.atan2(a1, a2))  
                
                
                
                currAngle = angle - 90
                if(currAngle < -180):
                    currAngle = 180 - (-180 - currAngle)
                    

               
                dev.currAngle = iirFilt(currAngle, dev.currAngle)
                            
                            
                #use dev2 yaw as the global yaw of system            
                if(dID == '2'):   
                    
                    if(dev.xAtThisIdx == 0):
                        xVect = rotateXvect(dev)
                        x = xVect[0]
                        y = xVect[1]                                                                                     
                        #gGlobalYaw_2 = math.degrees(math.asin(math.copysign(-1,y) * math.sqrt(1-pow(x,2)) ))          #temp = math.degrees(math.atan2(x,y))                                            
                        gGlobalYaw_2 = math.degrees(math.atan2(y, x))
                    else:
                        xVect = rotateYvect(dev)
                        x = xVect[0]
                        y = xVect[1]                                                                                                                                 
                        #gGlobalYaw_2 = math.degrees(math.asin(math.copysign(-1,x) * math.sqrt(1-pow(y,2)) ))          #temp = math.degrees(math.atan2(x,y))                                                
                        gGlobalYaw_2 = math.degrees(math.atan2(x, y / (math.sqrt(1 - pow(y,2))) ))
                        
                        
                        
                #use dev3 yaw as......
                if(dID == '3'):   
                    
                    if(dev.xAtThisIdx == 0):
                        xVect = rotateXvect(dev)
                        x = xVect[0]
                        y = xVect[1]                                                                                     
                        #gGlobalYaw_3 = math.degrees(math.asin( math.copysign(-1,y) * math.sqrt(1-pow(x,2)) ))          #temp = math.degrees(math.atan2(x,y))                                            
                        gGlobalYaw_3 = math.degrees(math.atan2(y, x))
                    else:
                        xVect = rotateYvect(dev)
                        x = xVect[0]
                        y = xVect[1]                                                                                     
                        #gGlobalYaw_3 = math.degrees(math.asin( math.copysign(-1,x) * math.sqrt(1-pow(y,2)) ))          #temp = math.degrees(math.atan2(x,y))  
                        gGlobalYaw_3 = math.degrees(math.atan2(x, y))
                
                    
                    logging.info("axis %f %f %f    %f %f  %d %d", x, y, z, gGlobalYaw_2, gGlobalYaw_3, gFoundDevices['2'].xAtThisIdx, gFoundDevices['3'].xAtThisIdx)
                    
                    
               
                    
            
            
            #plot device chain    [dev1]---[dev2]---[dev3]
            #base surface
            surf = pygame.Surface(rectSize, pygame.SRCALPHA)
            rect = surf.get_rect(center=rectSize)
            surf.fill((255,0,0))
            
            #search devices
            dev1 = findDev('1')
            dev2 = findDev('2')
            dev3 = findDev('3')
            
            
            #margin coeff to insert some space between rectangles
            margin = 1.05
            
            #device 1
            x = zeroPoint[0]
            y = zeroPoint[1]
            rot_UpperLeftCorner(x, y, surf, dev1.currAngle, rectSize[0])
                        
            #device 2  
            x = zeroPoint[0] + math.cos(math.radians(dev1.currAngle)) * (rectSize[0]*margin)
            y = zeroPoint[1] - math.sin(math.radians(dev1.currAngle)) * (rectSize[0]*margin)
            rot_UpperLeftCorner(x, y, surf, dev2.currAngle, rectSize[0])                                    
            
            #device 3
            x = zeroPoint[0] + math.cos(math.radians(dev1.currAngle)) * (rectSize[0]*margin) + math.cos(math.radians(dev2.currAngle)) * (rectSize[0]*margin)
            y = zeroPoint[1] - math.sin(math.radians(dev1.currAngle)) * (rectSize[0]*margin) - math.sin(math.radians(dev2.currAngle)) * (rectSize[0]*margin)
            rot_UpperLeftCorner(x, y, surf, dev3.currAngle, rectSize[0])
            
            
            #print angles            
            mess = "Angles [deg]:";            
            messageOnSceen(mess, 30, 30)
            
            mess = "dev1: {0:.1f}".format(dev1.currAngle)
            messageOnSceen(mess, 30, 70)
            
            mess = "dev2: {0:.1f}".format(dev2.currAngle)
            messageOnSceen(mess, 30, 100)
            
            mess = "dev3: {0:.1f}".format(dev3.currAngle)
            messageOnSceen(mess, 30, 130)
            
                        
        
        #update screen
        pygame.display.update()


def rot_center(image, rect, angle):
    """Rotate the image while keeping its center."""
    # Rotate the original image without modifying it.
    new_image = pygame.transform.rotate(image, angle)
    # Get a new rect with the center of the old rect.
    rect = new_image.get_rect(center=rect.center)
    return new_image, rect


def rot_UpperLeftCorner(x, y, surf, angle_deg, surf_width):
    
    rot_surf = pygame.transform.rotate(surf, angle_deg)

    angle_rad = math.radians(angle_deg)

    if((angle_deg >= 0) and (angle_deg <= 90)):
        screen.blit(rot_surf, (x, y - math.sin(angle_rad)*surf_width))                                  #primo quadrante
        
    elif((angle_deg < 0) and (angle_deg > -90)):
        screen.blit(rot_surf, (x, y))                                                               #quarto quadrante
    
    elif(angle_deg > 90) and (angle_deg <= 180):
        screen.blit(rot_surf, (x + math.cos(angle_rad)*surf_width, y - math.sin(angle_rad)*surf_width))     #secondo quadrante
        
    elif(angle_deg < -90) and (angle_deg >= -180):
        screen.blit(rot_surf, (x + math.cos(angle_rad)*surf_width, y))                                  #terzo quadrante



def iirFilt(x, y):
    alpha = 0.6
    y = x*alpha + y*(1-alpha)
    return y



# Get configuration info
parser = SafeConfigParser()
parser.read('PiHub.cfg')
_devicesToFind = parser.get('ble', 'devicesToFind')     # Only connect to devices advertising this name

logging.info("Looking for devices named: %s", _devicesToFind)


# Initialize Peripheral scanner
scanner = Scanner(0)




# Define some colors
BLACK = (0, 0, 0)
GRAY = (16, 16, 16)
WHITE = (255, 255, 255)
BLUE = (0, 0, 255)
GREEN = (0, 255, 0)
RED = (255, 0, 0)
 
PI = 3.141592653
gMilliTicks = 0
gZeroMilliTicks = 0
gGlobalYaw_2 = 0
gGlobalYaw_3 = 0

#flag and list of zero quaternion
gZeroDone = 0
gZeroQuaternions = {}

#init list of expected devices 
gExpectedDevices = {}
gExpectedDevices['1'] = (100,50)
gExpectedDevices['2'] = (200,50)
gExpectedDevices['3'] = (300,50)

#list of expexted devices
gFoundDevices = {}




# Initialize the game engine
pygame.init()
pygame.font.init()

# GUI inits
gWINDOW_SIZE = (900,900)                                #height and width of the screen
screen = pygame.display.set_mode(gWINDOW_SIZE)
gRECT_WIDTH = 40;
gRECT_HEIGHT = 140;
gLogoImge = pygame.image.load("TheTactigonTM_allAlphapng_gray_4.png")
gLogoImge = pygame.transform.scale(gLogoImge,(gWINDOW_SIZE[0], gWINDOW_SIZE[1]))
pygame.display.set_caption("Tactigon chain")



#create graphic thread
t = Graphichread()
logging.info("Starting Graphic Thread")
t.daemon = True
t.start()


#always scan for Tactigon BLE devices
done = False
while not done:
    devices = scanner.scan(2)
    for d in devices:
        try:
            # If scan returns a known addr that's already in the collection, it means it disconnected
            # Remove record and treat it as new                

            for (adtype, desc, value) in d.getScanData():                                                
                
                #search for Tactigon devices
                if(value.find(_devicesToFind) == 0):	
                    			
                    #get TACTI ID: are expected devices with name Test1, Test2, Test3, ....
                    tactiID = value[len(_devicesToFind):len(_devicesToFind)+1]
                    
                    if(tactiID in gExpectedDevices):

                        #create a device and put it in found devices list
                        gFoundDevices[tactiID] = TactiDevice(d.addr, tactiID)

                        #start thread
                        t = BleThread(d.addr, tactiID)                        
                        logging.info("Starting Thread for %s, tactiID %s", d.addr, tactiID)
                        t.daemon = True
                        t.start()
                    else:
                        logging.info("Unexpected Tacti with tactiID %s", tactiID)
                        
        except Exception, e:
            logging.debug("Unknown error %s", e.message)
            
        
            
    #handle windows closing
    for event in pygame.event.get():  # User did something
        if event.type == pygame.QUIT:  # If user clicked close
            done = True  # Flag that we are done so we exit this loop        

pygame.quit()
