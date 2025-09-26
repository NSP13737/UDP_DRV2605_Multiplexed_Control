#include "HapticPulser.h"
#include <cmath> // For std::round()

uint8_t HapticPulser::get_erm_od_clamp_reg(float v_erm_clamp_volts) {

    // --- Read timing registers ---
    uint8_t drive_reg = this->drv.readRegister8(DRV2605_REG_CONTROL1);    // 0x1B
    uint8_t ctrl2_reg = this->drv.readRegister8(DRV2605_REG_CONTROL2);    // 0x1C

    // DRIVE_TIME: bits 4–0, LSB = 0.5 ms
    float t_drive = (drive_reg & 0x1F) * 0.5e-3f; // seconds

    // DISS_TIME: bits 7–6
    float t_diss;
    switch ((ctrl2_reg >> 6) & 0x03) {
        case 0: t_diss = 0.0e-3f;   break;
        case 1: t_diss = 0.5e-3f;   break;
        case 2: t_diss = 1.0e-3f;   break;
        case 3: t_diss = 1.5e-3f;   break;
    }

    // BLANKING_TIME: bits 5–4
    float t_blank;
    switch ((ctrl2_reg >> 4) & 0x03) {
        case 0: t_blank = 0.0e-3f;   break;
        case 1: t_blank = 0.25e-3f;  break;
        case 2: t_blank = 0.5e-3f;   break;
        case 3: t_blank = 0.75e-3f;  break;
    }

    // --- Apply Equation 6 inverted ---
    float numerator   = v_erm_clamp_volts * (t_drive + t_diss + t_blank);
    float denominator = 21.33e-3f * (t_drive - 300e-6f);

    if (denominator <= 0.0f) {
        // Avoid divide-by-zero or negative
        return 0;
    }

    float reg_value = numerator / denominator;

    // Clamp to valid range
    if (reg_value < 0.0f)   reg_value = 0.0f;
    if (reg_value > 255.0f) reg_value = 255.0f;

    return static_cast<uint8_t>(std::round(reg_value));
}


bool HapticPulser::runAutoCal(float ratedVoltage, float odClamp) {
    uint8_t rated_voltage_reg = 0;
    uint8_t od_clamp_reg = 0;

    // 1) Configure motor type
    this->drv.useERM();
    rated_voltage_reg = (uint8_t)(ratedVoltage / 0.02133f); // convert from voltage to register value
    od_clamp_reg = get_erm_od_clamp_reg(odClamp);

    // 2) Set rated and clamp voltage
    this->drv.writeRegister8(DRV2605_REG_RATEDV, rated_voltage_reg); // reg 0x16
    this->drv.writeRegister8(DRV2605_REG_CLAMPV, od_clamp_reg);      // reg 0x17

    // 3) Configure feedback (FB_BRAKE_FACTOR, LOOP_GAIN, BEMF_GAIN)
    uint8_t fb = (0 << 7)               // N_ERM_LRA = 0 → ERM
                | (2 << 4)             // FB_BRAKE_FACTOR = 2 (recommended)
                | (2 << 2)             // LOOP_GAIN = 2 (recommended)
                | (2 << 0);            // BEMF_GAIN = 2
    this->drv.writeRegister8(DRV2605_REG_FEEDBACK, fb); // reg 0x1A

    // CONTROL1 (0x1B): DRIVE_TIME[4:0]
    // Each LSB = 0.5 ms. ERM motors: start with ~10 ms (20 * 0.5 ms)
    this->drv.writeRegister8(DRV2605_REG_CONTROL1, 20 & 0x1F);

    // NOTE: CONTROL2 changes below were suggested by AI, but this doesn't seem smart since the three
    //  registers it is trying to change are marked as advanced use only. Also this line is overriding
    //  the  BiDir_Input which we need to specify uni-directional
    // CONTROL2 (0x1C): IDISS_TIME, BLANK_TIME, SAMPLE_TIME
    //this->drv.writeRegister8(DRV2605_REG_CONTROL2, 0x17);

    // CONTROL4 (0x1D): AUTO_CAL_TIME[1:0]
    this->drv.writeRegister8(DRV2605_REG_CONTROL4, 0x03);


    // 4) Enter autocal mode
    // Store current mode to save for later
    uint8_t previousMode = (this->drv.readRegister8(DRV2605_REG_MODE)) & 0b111;
    this->drv.setMode(DRV2605_MODE_AUTOCAL);
    this->drv.go(); // start calibration

    // 5) Wait for completion
    while (this->drv.readRegister8(DRV2605_REG_GO) & 0x01) delay(10);

    // 6) Set board to previous mode
    this->drv.setMode(previousMode);
    delay(10); // short settle
        // Check to make sure mode has been set correctly 
    uint8_t mode_readback = this->drv.readRegister8(DRV2605_REG_MODE) & 0x07;
    if (mode_readback != previousMode) {
        Serial.printf("Warning: failed to restore mode: expected=%u read=%u\n", previousMode, mode_readback);
    }

    // 7) Check diag result
    uint8_t status = this->drv.readRegister8(DRV2605_REG_STATUS);
    bool diag_ok = !(status & (1 << 3)); // DIAG_RESULT = 0 => OK
    if (!diag_ok) return false;

    // 8) Optionally read compensation values
    uint8_t comp = this->drv.readRegister8(DRV2605_REG_AUTOCALCOMP); // 0x18
    uint8_t bemf = this->drv.readRegister8(DRV2605_REG_AUTOCALEMP);  // 0x19
    Serial.printf("AutoCal OK: comp=%u bemf=%u\n", comp, bemf);

    return true;
}
