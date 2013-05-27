from __future__ import division, print_function
import wallmask
import Image, ImageDraw
import struct

class Navmesh(object):
    JUMP_HEIGHT = 70
    STAIR_HEIGHT = 6
    MAX_STAIR_WIDTH = 11*6

    def __init__(self):
        self.wallmask = wallmask.Wallmask()
        width = 18
        height = 35
        speed = 2
        #self.generate_navmesh(width, height, speed)
        #self.save_navmesh()
        self.mesh = []
        self.load_navmesh(self.wallmask.name+".navmesh")
        self.draw_navmesh()
        print("---DONE---")

    def generate_navmesh(self, char_width, char_height, char_speed):
        mask = self.wallmask.mask
        self.mesh = []
    
        # Generate all areas
        print("---GENERATING AREAS---")
        for y in range(self.wallmask.height-1):
            x = 0
            while x < self.wallmask.width-1:
                if (not mask[x][y]) and mask[x][y+1]:
                    if x != 0 and y != 0:
                        i = x
                        j = y
                        max_height = -1
                        while True:
                            # Go through all pixels of the future rect, we have time
                            j -= 1
                            
                            if mask[i][j]:
                                #print("MASK HIT ALERT")
                                # We've hit wallmask
                                if max_height == -1:
                                    # If we don't know how high we're supposed to go yet (first run)
                                    max_height = y-j
                                    if max_height < 0:
                                        print("Error 1 in generation! Max_height = {0}, x = {1}, y = {2}, i = {3}, j = {4}".format(max_height, x, y, i, j))
                                    # Move one row to the right
                                    i += 1
                                    j = y

                                elif max_height > y - j:
                                    # If we do, and we hit wallmask before we were supposed to, create a rect and stop this one
                                    r = self.create_rect(x, y, i-x-1, max_height-1)
                                    break
                            
                            if (y - j) == self.JUMP_HEIGHT+char_height and max_height == -1:
                                max_height = self.JUMP_HEIGHT+char_height
                            
                            if (y - j) == max_height:
                                if mask[i][j-1] or (y - j) == self.JUMP_HEIGHT+char_height:
                                    i += 1
                                    j = y
                                    if (not mask[i][j+1]) or mask[i][j]:
                                        r = self.create_rect(x, y, i-x-1, max_height-1)
                                        break
                                else:
                                    r = self.create_rect(x, y, i-x-1, max_height-1)
                                    break
                            
                            if i == self.wallmask.width:
                                # max_height will never be -1 here
                                r = self.create_rect(x, y, i-x, max_height-1)
                            
                            if j == 0:
                                if max_height == -1:
                                    max_height = y - j
                                    if max_height < 0:
                                        print("Error 2 in generation! Max_height = {0}, x = {1}, y = {2}, i = {3}, j = {4}", max_height, x, y, i, j)
                                else:
                                    r = self.create_rect(x, y, i-x, max_height-1)
                                    break
                        
                        self.mesh.append(r)
                        if i == x:
                            print("Stop! Error: i == x!", i, x, y, j)
                            raw_input()
                        x = i-1
                x += 1
                

        # Areas are only supposed to be visitable by one point of the character, the left-bottom one
        # Make them smaller to account for that
        i = 0
        while i < len(self.mesh):
            # Go through all rectangles, but skip over those that were deleted (hence while and not for)
            rect = self.mesh[i]
            rect.topleft.y += char_height
            rect.topright.y += char_height
            if rect.topleft.y >= rect.bottomleft.y or rect.topright.y >= rect.bottomright.y:
                self.mesh.remove(rect)
            else:
                i += 1
        
        
        # Stair optimizations
        print("---OPTIMIZING STAIRS---")
        i = 0
        while i < len(self.mesh):
            # Go through all rectangles, but skip over those that were deleted (hence while and not for)
            rect = self.mesh[i]
            # The rectangle would only be a stair if it's width is the width of one step
            stair_width = rect.bottomright.get_coord()[0] - rect.bottomleft.get_coord()[0] + 1
            if stair_width in range(1, self.MAX_STAIR_WIDTH):
                # If it's small enough that it can be called a stair
                x, y = rect.bottomleft.get_coord()
                # Find where the next step /should/ be
                next_rect = self.find_rect(x-stair_width, y+self.STAIR_HEIGHT)
                if next_rect != None:
                    # Stair going left and down
                    while next_rect != None:
                        # Check whether the stair continues
                        if next_rect.bottomright.x - next_rect.bottomleft.x + 1 != stair_width:
                            break
                        if rect.topleft.y - rect.bottomleft.y != next_rect.topright.y - next_rect.bottomright.y:
                            break
                        
                        # As long as steps are where they should
                        # Merge them into the original one
                        rect.bottomleft = next_rect.bottomleft
                        rect.topleft = next_rect.topleft
                        self.mesh.remove(next_rect)
                        x -= stair_width
                        y += self.STAIR_HEIGHT
                        next_rect = self.find_rect(x-stair_width, y+self.STAIR_HEIGHT)
                else:
                    # Same as above, only for right and down
                    x, y = rect.bottomleft.get_coord()
                    next_rect = self.find_rect(x+stair_width, y+self.STAIR_HEIGHT)
                    while next_rect != None:
                        # Check whether the stair continues
                        if next_rect.bottomright.get_coord()[0] - next_rect.bottomleft.get_coord()[0] + 1 != stair_width:
                            break
                        if rect.topright.get_coord()[1] - rect.bottomright.get_coord()[1] != next_rect.topleft.get_coord()[1] - next_rect.bottomleft.get_coord()[1]:
                            break
                        rect.bottomright = next_rect.bottomright
                        rect.topright = next_rect.topright
                        self.mesh.remove(next_rect)
                        x += stair_width
                        y += self.STAIR_HEIGHT
                        next_rect = self.find_rect(x+stair_width, y+self.STAIR_HEIGHT)
            i += 1
        
        
        # Connect touching areas
        print("---CONNECTING NEIGHBOURING AREAS---")
        for rect in self.mesh:
            for other_rect in self.mesh:
                if other_rect == rect or other_rect in rect.connections:
                    # No need to compute any kind of connection
                    continue
                if other_rect.bottomright.get_coord()[0] == rect.bottomleft.get_coord()[0] - 1:
                    # other_rect is left of the current rect
                    # Check if the y's connect, which means that the lines from topleft<->bottomright & topright<->bottomleft cross, ie. they have the same slope signs
                    sign1 = sign(rect.topleft.get_coord()[1] - other_rect.bottomright.get_coord()[1])
                    sign2 = sign(other_rect.topright.get_coord()[1] - rect.bottomleft.get_coord()[1])
                    if sign1 == sign2 or sign1 == 0 or sign2 == 0:
                        self.connect_rects_mutually(rect, other_rect)
                elif other_rect.bottomleft.get_coord()[0] == rect.bottomright.get_coord()[0] + 1:
                    # other_rect is right of the current rect
                    # Check if the y's connect, which means that the lines from topleft<->bottomright & topright<->bottomleft cross, ie. they have the same slope signs
                    sign1 = sign(other_rect.topleft.get_coord()[1] - rect.bottomright.get_coord()[1])
                    sign2 = sign(rect.topright.get_coord()[1] - other_rect.bottomleft.get_coord()[1])
                    if sign1 == sign2 or sign1 == 0 or sign2 == 0:
                        self.connect_rects_mutually(rect, other_rect)

        # Connect with jumping
        print("---CONNECTING OTHER AREAS THROUGH BRUTE-FORCE---")
        sim = Simulation(self, char_width, char_height, char_speed)
        i = 0
        for rect in self.mesh:
            if int(100*i/len(self.mesh)) > int(100*(i-1)/len(self.mesh)):
                print("{0}%".format(int(100*i/len(self.mesh))))
            i += 1
            sim.simulate(rect)

    def find_rect(self, x, y):
        pos = (x, y)
        for rect in self.mesh:
            if rect.bottomleft.get_coord() == pos:
                return rect
        return None
    
    def create_rect(self, x, y, width, height):
        r = Polygon()
        r.bottomleft.set_coord(x, y)
        r.bottomright.set_coord(x+width, y)
        r.topleft.set_coord(x, y-height)
        r.topright.set_coord(x+width, y-height)
        print("\nNew rect:")
        print(r.topleft.get_coord())
        print(r.bottomright.get_coord())
        raw_input()
        return r
    
    def connect_rects_mutually(self, rect1, rect2):
        rect1.connections.append(rect2)
        rect2.connections.append(rect1)
    
    def draw_navmesh(self):
        print("---DRAWING FINISHED NAVMESH---")
    
        im = Image.new("RGB", (self.wallmask.width, self.wallmask.height))
        
        for x in range(self.wallmask.width):
            for y in range(self.wallmask.height):
                im.putpixel((x, y), (255, 255, 255))
                
        for x in range(self.wallmask.width):
            for y in range(self.wallmask.height):
                if self.wallmask.mask[x][y]:
                    im.putpixel((x, y), (0, 0, 0))
        
        draw = ImageDraw.Draw(im)
        for rect in self.mesh:
            draw.line([rect.bottomleft.get_coord(), rect.bottomright.get_coord()], (0, 212, 255), width=1)
            draw.line([rect.topleft.get_coord(), rect.topright.get_coord()], (0, 212, 255), width=1)
            draw.line([rect.bottomleft.get_coord(), rect.topleft.get_coord()], (0, 212, 255), width=1)
            draw.line([rect.bottomright.get_coord(), rect.topright.get_coord()], (0, 212, 255), width=1)
            for r in rect.connections:
                pos1 = (int(rect.bottomright.get_coord()[0]/2 + rect.bottomleft.get_coord()[0]/2), int(rect.bottomright.get_coord()[1]/2 + rect.topleft.get_coord()[1]/2))
                pos2 = (int(r.bottomright.get_coord()[0]/2 + r.bottomleft.get_coord()[0]/2), int(r.bottomright.get_coord()[1]/2 + r.topleft.get_coord()[1]/2))
                draw.line([pos1, pos2], (200, 0, 0), width=1)
                
                direction = (pos1[0] + int((pos2[0]-pos1[0])/5), pos1[1] + int((pos2[1]-pos1[1])/5))
                draw.line([pos1, direction], (0, 0, 200), width=3)

        im.save("output.png")
    
    def save_navmesh(self):
        print("---EXPORTING NAVMESH---")
        data = struct.pack("<H", len(self.mesh))
        for rect in self.mesh:
            data += struct.pack("<II", rect.topleft.get_coord()[0], rect.topleft.get_coord()[1])
            data += struct.pack("<II", rect.bottomleft.get_coord()[0], rect.bottomleft.get_coord()[1])
            data += struct.pack("<II", rect.topright.get_coord()[0], rect.topright.get_coord()[1])
            data += struct.pack("<II", rect.bottomright.get_coord()[0], rect.bottomright.get_coord()[1])
        
        for rect in self.mesh:
            data += struct.pack("<B", len(rect.connections))
            for r in rect.connections:
                data += struct.pack("<H", self.mesh.index(r))
        
        f = open(self.wallmask.name+".navmesh", "wb")
        f.write(data)
        f.close()
    
    def load_navmesh(self, fname):
        print("---LOADING NAVMESH---")
        f = open(fname, "rb")
        data = f.read()
        size = struct.unpack_from("<I", data)[0]
        data = data[4:]
        self.mesh = []
        # Positions
        for i in range(size):
            p = Polygon()
            
            
            x, y = struct.unpack_from("<II", data)
            p.topleft.set_coord(x, y)
            data = data[8:]
            x, y = struct.unpack_from("<II", data)
            p.bottomleft.set_coord(x, y)
            data = data[8:]
            x, y = struct.unpack_from("<II", data)
            p.topright.set_coord(x, y)
            data = data[8:]
            x, y = struct.unpack_from("<II", data)
            p.bottomright.set_coord(x, y)
            data = data[8:]
            
            self.mesh.append(p)
        
        '''for rect in self.mesh:
            # Connections
            n_connections = struct.unpack_from("<I", data)[0]
            data = data[1:]
            for j in range(n_connections):
                rect.connections.append(self.mesh[struct.unpack_from("<I", data)[0]])
                data = data[2:]'''

        

class Simulation(object):
    SIMULATION_GRANULARITY = 10
    def __init__(self, navmesh, char_width, char_height, char_speed):
        self.char_x = 0
        self.char_y = 0
        self.char_hs = 0
        self.char_vs = 0
        self.char_width = char_width
        self.char_height = char_height
        self.char_speed = char_speed# RunPower in gg2 source
        self.navmesh = navmesh
        self.gravity = 0.6
        self.fps_acceleration = 1
    
    def simulate(self, rect):
        if rect.bottomleft.get_coord()[1] != rect.bottomright.get_coord()[1]:
            # TODO: Stairs
            return
        
        for direction in [-6, 6]:
            if direction == 6:
                start_x, tmp_y = rect.bottomright.get_coord()
            else:
                start_x, tmp_y = rect.bottomleft.get_coord()
                start_x -= self.char_width
            
            # Do this first for the left side, then for the right side
            for start_y, start_vs in [(tmp_y, 0), (tmp_y, -8*self.fps_acceleration), (tmp_y - (rect.bottomleft.get_coord()[1] - rect.topleft.get_coord()[1]), 0)]:
                # Do this walking off the platform, jumping off, and being at the apex of a jump
                for input_dir in [-1, 1]:
                    # Try all two input extremes
                    self.char_x = start_x+direction
                    self.char_y = start_y
                    self.char_hs = input_dir * self.char_speed * self.fps_acceleration
                    self.char_vs = start_vs
                    if self.collides_with_wallmask():
                        # If there's a wall in our path, we can already give up
                        break
                    while True:
                        break_out = False
                        # Apply gravity
                        self.char_vs += self.gravity*self.fps_acceleration
                        self.char_vs = min(self.char_vs, 10)
                        # Move carefully, while checking for collisions
                        for i in range(self.SIMULATION_GRANULARITY):
                            self.char_x += self.char_hs/self.SIMULATION_GRANULARITY
                            self.char_y += self.char_vs/self.SIMULATION_GRANULARITY
                            
                            # Check whether we've landed in an area
                            new_rect = self.collides_with_navmesh()
                            if new_rect != None:
                                # We landed somewhere, find out whether we already know this one
                                if new_rect not in rect.connections and new_rect != rect:
                                    # Connect it
                                    rect.connections.append(new_rect)
                                break_out = True
                                break

                            # Respond to collisions with wallmask
                            # We can't collide vertically with anything as long as we check for areas first
                            counter = 0
                            while self.collides_with_wallmask():
                                self.char_x -= self.char_hs/self.SIMULATION_GRANULARITY
                                counter += 1
                                if counter > self.SIMULATION_GRANULARITY:
                                    break_out = True
                                    break
                            
                            if break_out:
                                break
                        if break_out:
                            break

    def collides_with_wallmask(self):
        collided = False
        for w in range(self.char_width):
            for h in range(self.char_height):
                if self.navmesh.wallmask.mask[int(self.char_x + w)][int(self.char_y - h)]:
                    collided = True
                    break
            if collided:
                break
        return collided
                
    def collides_with_navmesh(self):
        for rect in self.navmesh.mesh:
            if rect.bottomleft.get_coord()[0] > self.char_x + self.char_width:
                continue
            if rect.bottomright.get_coord()[0] < self.char_x:
                continue
            factor = (self.char_x - rect.bottomleft.get_coord()[0]) / (rect.bottomright.get_coord()[0] - rect.bottomleft.get_coord()[0])
            y_top = rect.topleft.get_coord()[1] + factor*(rect.topright.get_coord()[1] - rect.topleft.get_coord()[1])
            y_bottom = rect.bottomleft.get_coord()[1] + factor*(rect.bottomright.get_coord()[1] - rect.bottomleft.get_coord()[1])
            if self.char_y <= y_top or self.char_y - self.char_height >= y_bottom:
                continue
            return rect
        return None

class Point(object):
    def set_coord(self, x, y):
        self.x = x
        self.y = y
    
    def get_coord(self):
        return (self.x, self.y)

class Polygon(object):
    def __init__(self):
        self.bottomleft = Point()
        self.bottomright = Point()
        self.topleft = Point()
        self.topright = Point()
        self.connections = []

def sign(x):
    if x > 0:
        return 1
    elif x < 0:
        return -1
    else:
        return 0

n = Navmesh()
