# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is a documentation repository for MT门压芯片 (MT Door Pressure Chip) automated testing system. The repository contains Chinese language technical documentation for developing an automated testing solution for pressure sensor chips (specifically SMP475) using PSI5 communication protocol.

## Repository Structure

The repository is organized as follows:

- **Documentation Files:**
  - `SoftwareRequirementsDocument.md` - Complete software requirements specification in Chinese
  - `SRD.md` - Software requirements document with detailed technical specifications
  - `0806backUp.md` - Backup documentation file
  - `test.md` - Test file (appears to be corrupted)

- **Images:**
  - `specs/images/` - Technical diagrams and flowcharts used by the requirements document
  - Various process flow diagrams including calibration, OTP writing, and error verification processes

## Key Project Context

This project involves:
- **Target Chip:** SMP475 pressure sensor chip
- **Communication:** PSI5 protocol (Peripheral Sensor Interface 5)
- **Testing Requirements:** 4 temperature zones, pressure range ≤300KPa
- **Performance Goal:** UPH (Units Per Hour) ≥1200
- **System:** Automated chip handling, calibration, OTP writing, and error verification

## Technical Details

### Core Testing Processes
1. **Calibration Process** - Multi-temperature, multi-pressure point testing
2. **OTP Writing Process** - Writing calibration results to chip OTP area
3. **Error Verification Process** - Validating chip performance after calibration

### Key External Tools Referenced
- `MTPTCali.exe` - Customer-provided calibration tool
- `MTPTCheck.exe` - Customer-provided verification tool
- AP8000 devices for multi-channel testing (16 channels total)

### Data Management
- CSV file formats for sample data and OTP results
- Cache folder management (D:\MT\Cache)
- Naming conventions: `DUT-X-Sample.csv`, `DUT-X-OTPResult.csv`, `DUT-X-TestSample.csv`

## Working with This Repository

### Common Tasks
- **Document editing:** All documents are in Markdown format with Chinese content
- **Image management:** Technical diagrams are stored in the `image/` subdirectory
- **Documentation updates:** Focus on maintaining consistency between SRD.md and SoftwareRequirementsDocument.md

### File Relationships
- `SoftwareRequirementsDocument.md` appears to be the most current and comprehensive document
- `SRD.md` contains additional technical specifications and interface details
- Both documents should be kept synchronized when making updates

### Language Considerations
- Primary language is Chinese (Simplified)
- Technical terms mix Chinese and English (e.g., "门压芯片", "PSI5", "UPH")
- Maintain language consistency when making edits

## Development Context

This is a documentation-only repository for a hardware testing system. When working with these files:
- Preserve Chinese language content and formatting
- Maintain technical accuracy for hardware specifications
- Keep process flow descriptions consistent with accompanying diagrams
- Ensure all referenced file paths and naming conventions remain accurate