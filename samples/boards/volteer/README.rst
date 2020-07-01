.. _volteer:

volteer
###########

Overview
********

A simple sample that can be used to boot Google CRD (Chrome Reference Design)
Volteer with Non-Deep Sx configuration.

Building and Running
********************

This application can be built and executed on QEMU as follows:

.. zephyr-app-commands::
   :zephyr-app: samples/boards/tglrvp
   :host-os: unix
   :board: qemu_x86
   :goals: run
   :compact:

To build for another board, change "qemu_x86" above to that board's name.

Sample Output
=============

.. code-block:: console

    Look for prints about the power rails

Exit QEMU by pressing :kbd:`CTRL+A` :kbd:`x`.
