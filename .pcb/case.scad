$fn = 256;

// ==========================================
// CONFIGURATION
// ==========================================

RENDER_PART = "both"; 

// ==========================================
// PARAMETERS
// ==========================================

// THICKNESS SETTINGS
T = 2.0;                // Now 2mm for strength
BOTTOM_T = 4.0;         

// VERTICAL FIXTURES
ORIGINAL_REQ_STANDOFF = 10; 
FIXED_PCB_Z = T + ORIGINAL_REQ_STANDOFF; 

// PCB DIMENSIONS
PCB_W = 100.0;
PCB_L = 200.0;
PCB_T = 1.6;
PCB_CORNER_R = 9.0;     // Exact radius of the PCB corners

// HOUSING DIMENSIONS
H = 40.0;               
SCREW_HOLE_R = 1.4;     
PCB_HOLE_OFFSET = 10.0; 

// SNAP FIT PARAMETERS
SNAP_Y_LOCS = [40, PCB_W + (2*T) - 40]; 
SNAP_Z_POS = H - 3.0;                   
SNAP_RADIUS = 1.3;                      // Slightly larger for better click
HOLE_WIDTH = 5.0;                       
HOLE_HEIGHT = 3.0;                      

// USB PORT PARAMETERS
USB_WIDTH = 12;         
USB_HEIGHT = 6;         

// WAGO CONNECTOR PARAMETERS
WAGO_WIDTH = 12;
WAGO_HEIGHT = 8;
wago_x_positions = [25, 175]; 

// LID & LED PARAMETERS
LID_T = 2.0;            
LIP_H = 6.0;            // Deeper lip for T=2mm stability
TOLERANCE = 0.25;       // Slightly more tolerance for thicker walls
BAFFLE_W = 6.0;         
BAFFLE_H = (H - FIXED_PCB_Z - PCB_T - TOLERANCE); 

// LED CONFIGURATION
LED_COUNT = 7;
LED_PITCH = 5.0;        
LED_HOLE_R = 1.5;       
LED_X_ON_PCB = 42.5;    
LED_Y_START_USER = -30.0; 

// ==========================================
// DERIVED VALUES
// ==========================================

W = (PCB_W + (2 * T));    
L = (PCB_L + (2 * T));
// Outer Radius = PCB Radius + Wall Thickness
OUTER_R = PCB_CORNER_R + T; 

PCB_ORIGIN_X_SCAD = T;
PCB_ORIGIN_Y_SCAD = T + PCB_W;

// ==========================================
// HELPER MODULES
// ==========================================

module rounded_rect_2d_tl(w, h, r) {
    translate([r, r]) offset(r=r) square([w - 2*r, h - 2*r], center=false);
}

module outer_block(w, h, t, r) {
    linear_extrude(height=t) rounded_rect_2d_tl(w, h, r);
}

module capsule_hole(length, width) {
    r = width / 2;
    hull() {
        translate([0, 0]) circle(r = r);
        translate([length, 0]) circle(r = r);
    }
}

// ==========================================
// MAIN HOUSING MODULE
// ==========================================

module housing() {
    x_min = T + PCB_HOLE_OFFSET;
    x_max = T + PCB_L - PCB_HOLE_OFFSET;
    y_min = T + PCB_HOLE_OFFSET;
    y_max = T + PCB_W - PCB_HOLE_OFFSET;
    
    post_positions = [[x_min, y_min], [x_max, y_min], [x_max, y_max], [x_min, y_max]];
    slot_positions = [[30, 75], [30, 25], [170, 75], [170, 25]];

    difference() {
        // BASE BLOCK (Uses Calculated OUTER_R)
        outer_block(L, W, H, OUTER_R);
        
        // INTERIOR CUTOUT (Uses exact PCB_CORNER_R)
        translate([T, T, BOTTOM_T])
            linear_extrude(height = H - BOTTOM_T + 0.05)
                rounded_rect_2d_tl(PCB_L, PCB_W, PCB_CORNER_R);
        
        // WALL MOUNT SLOTS
        for (pos = slot_positions) {
            translate([pos.x, pos.y, -1])
                linear_extrude(height = BOTTOM_T + 2) capsule_hole(10, 3);
        }

        // WAGO CUTOUTS
        for (x_pos = wago_x_positions) {
            translate([x_pos + T, W - (T/2), FIXED_PCB_Z + (WAGO_HEIGHT/2) + 1]) 
                rotate([90, 0, 0]) 
                    linear_extrude(height = T + 10, center=true)
                        hull() {
                            w_half = WAGO_WIDTH/2 - 2; h_half = WAGO_HEIGHT/2 - 2;
                            translate([-w_half, -h_half]) circle(r=2);
                            translate([ w_half, -h_half]) circle(r=2);
                            translate([ w_half,  h_half]) circle(r=2);
                            translate([-w_half,  h_half]) circle(r=2);
                        }
        }
        
        // USB CUTOUT
        translate([
            T + (PCB_L / 2), 
            W - (T / 2), 
            FIXED_PCB_Z + PCB_T + (USB_HEIGHT / 2) 
        ])
            rotate([90, 0, 0]) 
                linear_extrude(height = T + 10, center=true)
                    hull() {
                        w_half = USB_WIDTH/2 - 1; 
                        h_half = USB_HEIGHT/2 - 1;
                        translate([-w_half, -h_half]) circle(r=1);
                        translate([ w_half, -h_half]) circle(r=1);
                        translate([ w_half,  h_half]) circle(r=1);
                        translate([-w_half,  h_half]) circle(r=1);
                    }
                    
        // SNAP HOLES
        for (y_pos = SNAP_Y_LOCS) {
            translate([-1, y_pos - (HOLE_WIDTH/2), SNAP_Z_POS - (HOLE_HEIGHT/2)])
                cube([T + 2, HOLE_WIDTH, HOLE_HEIGHT]);
            
            translate([L - T - 1, y_pos - (HOLE_WIDTH/2), SNAP_Z_POS - (HOLE_HEIGHT/2)])
                cube([T + 2, HOLE_WIDTH, HOLE_HEIGHT]);
        }
    }

    // STANDOFFS
    for (pos = post_positions) {
        translate([pos.x, pos.y, 0])
            difference() {
                cylinder(h = FIXED_PCB_Z, r = 5);
                translate([0, 0, -1]) cylinder(h = FIXED_PCB_Z + 2, r = SCREW_HOLE_R);
            }
    }
}

// ==========================================
// LID MODULE
// ==========================================

module lid() {
    y_start_scad = PCB_ORIGIN_Y_SCAD + LED_Y_START_USER;
    led_array_len = (LED_COUNT - 1) * LED_PITCH;
    baffle_total_len = led_array_len + 6; 
    baffle_center_y = y_start_scad - (led_array_len / 2);

    difference() {
        union() {
            // LID PLATE
            linear_extrude(height = LID_T) rounded_rect_2d_tl(L, W, OUTER_R);
            
            // LID LIP (Matches PCB Radius minus tolerance)
            translate([T + TOLERANCE, T + TOLERANCE, -LIP_H])
                linear_extrude(height = LIP_H)
                     rounded_rect_2d_tl(
                        L - 2*T - 2*TOLERANCE, 
                        W - 2*T - 2*TOLERANCE, 
                        PCB_CORNER_R - TOLERANCE
                     );
                    
            // LIGHT BAFFLE
            translate([
                PCB_ORIGIN_X_SCAD + LED_X_ON_PCB - (BAFFLE_W/2), 
                baffle_center_y - (baffle_total_len / 2), 
                -BAFFLE_H
            ])
                cube([BAFFLE_W, baffle_total_len, BAFFLE_H]);
                
            // SNAP BUMPS
            snap_z_local = -(H - SNAP_Z_POS);
            
            for (y_pos = SNAP_Y_LOCS) {
                translate([T + TOLERANCE, y_pos, snap_z_local])
                    rotate([0, 90, 0]) sphere(r=SNAP_RADIUS);
                            
                translate([L - T - TOLERANCE, y_pos, snap_z_local])
                    rotate([0, 90, 0]) sphere(r=SNAP_RADIUS);
            }
        }

        // LED HOLES
        for (i = [0 : LED_COUNT - 1]) {
            posX = PCB_ORIGIN_X_SCAD + LED_X_ON_PCB;
            posY = PCB_ORIGIN_Y_SCAD + LED_Y_START_USER - (i * LED_PITCH);
            translate([posX, posY, -BAFFLE_H - 1]) 
                cylinder(h = LID_T + BAFFLE_H + 2, r = LED_HOLE_R);
        }
    }
}

// ==========================================
// RENDER LOGIC
// ==========================================

if (RENDER_PART == "box" || RENDER_PART == "both") {
    color("LightBlue") housing();
}

if (RENDER_PART == "lid" || RENDER_PART == "both") {
    translate([L + 20, W, 0]) 
        rotate([180, 0, 0]) 
            translate([0, 0, -LID_T]) 
                color("Orange") lid();
}