# Linux Keylogger Study

## Overview

This project is a personal experimental exercise developed to explore low-level Linux programming, keyboard input handling, and client-server communication.

The main objective was to gain practical experience with Linux input devices and system interfaces, while investigating how keyboard events can be accessed, processed, and transmitted between different components.

The implementation consists of a C client and a Python server.

## Learning Objectives

- Explore the Linux input subsystem and keyboard event handling.
- Experiment with C system programming and Linux APIs.
- Understand TCP socket communication between a client and a server.
- Study the interaction between processes, system privileges, and system services.
- Examine techniques associated with keylogging malware from a technical and educational perspective.

## Implementation and Limitations

This implementation is an experimental learning exercise, not a production-ready tool. It has several technical and portability limitations, including dependencies on Linux-specific interfaces, device permissions, system configuration, and keyboard event representation.

The keyboard mapping is limited and may not correctly represent all layouts or input conditions. The implementation should not be considered reliable, secure, or compatible with every Linux environment or operating system.

The data transformation used for transmission is a basic XOR operation, not secure encryption.

## Security Considerations

The original implementation includes functionality related to keystroke collection, password capture, network transmission, and system service persistence. These are sensitive capabilities that can be abused to compromise privacy and system security.

This repository documents the project for educational and portfolio purposes. It is intended to demonstrate my learning process and technical exploration, not to encourage unauthorized access, credential collection, or malicious use.

Any experimentation should take place only in an isolated environment on systems for which explicit authorization has been obtained. The code should not be deployed on other people's systems or used to collect real credentials or personal information.

## Project Status

This project is an educational experiment and is not actively developed as a deployable tool. Its limitations and implementation choices are part of the learning experience documented in this repository.