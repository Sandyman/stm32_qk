//$file${Src::blink.cpp} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
//
// Model: blink.qm
// File:  ${Src::blink.cpp}
//
// This code has been generated by QM 5.2.1 <www.state-machine.com/qm>.
// DO NOT EDIT THIS FILE MANUALLY. All your changes will be lost.
//
// SPDX-License-Identifier: GPL-3.0-or-later
//
// This generated code is open source software: you can redistribute it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation.
//
// This code is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
// more details.
//
// NOTE:
// Alternatively, this generated code may be distributed under the terms
// of Quantum Leaps commercial licenses, which expressly supersede the GNU
// General Public License and are specifically designed for licensees
// interested in retaining the proprietary status of their code.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//
//$endhead${Src::blink.cpp} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
#include "blink.hpp"
#include "events.hpp"
#include "bsp.hpp"
#include "glow.h"
#include "utils.hpp"

Q_DEFINE_THIS_FILE

//$skip${QP_VERSION} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Check for the minimum required QP version
#if (QP_VERSION < 690U) || (QP_VERSION != ((QP_RELEASE^4294967295U) % 0x3E8U))
#error qpcpp version 6.9.0 or higher required
#endif
//$endskip${QP_VERSION} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${Components::Blink} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Components::Blink} .......................................................
Blink Blink::inst __CCMRAM;


//${Components::Blink::Blink} ................................................
Blink::Blink()
 : QActive(&initial),
   _TimeEvt(this, TIMEOUT_SIG, 0U)
{}

//${Components::Blink::SM} ...................................................
Q_STATE_DEF(Blink, initial) {
    //${Components::Blink::SM::initial}
    return tran(&blink);
}

//${Components::Blink::SM::blink} ............................................
Q_STATE_DEF(Blink, blink) {
    QP::QState status_;
    switch (e->sig) {
        //${Components::Blink::SM::blink::initial}
        case Q_INIT_SIG: {
            status_ = tran(&on);
            break;
        }
        default: {
            status_ = super(&top);
            break;
        }
    }
    return status_;
}

//${Components::Blink::SM::blink::off} .......................................
Q_STATE_DEF(Blink, off) {
    QP::QState status_;
    switch (e->sig) {
        //${Components::Blink::SM::blink::off}
        case Q_ENTRY_SIG: {
            BSP::UserLed.set_low();

            _TimeEvt.armX(msec(20));
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Components::Blink::SM::blink::off}
        case Q_EXIT_SIG: {
            _TimeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Components::Blink::SM::blink::off::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&on);
            break;
        }
        default: {
            status_ = super(&blink);
            break;
        }
    }
    return status_;
}

//${Components::Blink::SM::blink::on} ........................................
Q_STATE_DEF(Blink, on) {
    QP::QState status_;
    switch (e->sig) {
        //${Components::Blink::SM::blink::on}
        case Q_ENTRY_SIG: {
            const uint8_t angle_difference = 11;
            static uint8_t angle = 0;

            BSP::UserLed.set_high();

            for (uint_fast8_t i = 0; i < NUM_PIXELS; i++)
            {
               // Calculate color
               uint32_t rgb_color = hsl_to_rgb(angle + (i * angle_difference), 255, 31);

               // Set color
               glow_set_RGB(i, rgb_color);
            }

            angle += 2;

            // Write to LED
            glow_render();

            _TimeEvt.armX(msec(20));
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Components::Blink::SM::blink::on}
        case Q_EXIT_SIG: {
            _TimeEvt.disarm();
            status_ = Q_RET_HANDLED;
            break;
        }
        //${Components::Blink::SM::blink::on::TIMEOUT}
        case TIMEOUT_SIG: {
            status_ = tran(&off);
            break;
        }
        default: {
            status_ = super(&blink);
            break;
        }
    }
    return status_;
}
//$enddef${Components::Blink} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//$define${Components::AO_Blink} vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

//${Components::AO_Blink} ....................................................
QP::QActive * const AO_Blink  = &Blink::inst;
//$enddef${Components::AO_Blink} ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
