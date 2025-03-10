/** @file       main.h
  * @version    1.0.0
  * @author     Minhneo
  * @date       1/2023
  * @brief      
  */

/**
   \mainpage Incense Vending Machine TPA Library
   *
   * Introduction
   * ------------
   *
   * Huong dan nay mo ta cac thu vien firmware cua may ban huong TPA
   * mot trong cac san pham nghien cuu va phat trien dua tren phan cung may ban hang tu dong
   *
   * Thu vien bao gom mot so chuc nang dieu khien ngoai vi nhu sau:
   * - Ket noi GSM
   * - Ket noi VTL
   * - Doc va dieu khien do am bang cam bien do am DHT21
   * - Luu tru ban tin trong E2P
   * - Ket noi cam bien cua
   * - Cap nhat thoi gian thuc va quan ly thoi gian thuc theo 2 phuong thuc (RTC, SERVERtime)
   * - Debug multi level
   * - Watchdog
   * - AES encryption
   *
   *
   * Using the Library
   * ------------
   *
   * Bo thu vien bao gom cac thu muc trong  <code>Source</code> folder.
   * Thu vien duoc chia lam 3 phan co ban cua 1 frimware hoan chinh bao gom
   * - App - Quan ly hoat dong he thong theo chu ky 10, 100, 500, 1000, 5000 ms
   * - Service - Quan ly giao tiep dieu khien ngoai vi, thu thap du lieu ngoai vi
   * - Driver - Giao tiep truc tiep voi ngoai vi, Khai bao du lieu co ban
   * 
   * 
   * Cac dia chi luu tru du lieu trong e2p duoc khai bao trong <code>peripheral.h</code> trong thu muc <code>Include</src/library/</code>
   * Cac muc debug duoc khai bao trong <code>dbg_file_level.h</code> trong thu muc <code>Include</src/library/service/inc></code>
   *
   *
   * Examples
   * --------
   *
   * Thu vien nay khong cung cap cac vi du minh hoa - Day la thu vien ung dung
   *
   * Toolchain Support
   * ------------
   *
   * Thu vien duoc phat trien va thu nghiem voi IAR 8.40.2
   * 
   *
   * <hr>
   * Incense Vending Machine Library Workspace Struct
   * -----------------------------
   *
   * Cau truc thu vien trong IAR EW nhu sau:
   * |File/Folder                    |Content                                                                 |
   * |-------------------------------|------------------------------------------------------------------------|
   * |\b src\\App                    | Luu tru chuong trinh chinh, he thong menu, du lieu hien thi man hinh   |
   * |\b src\\library\\driver        | Driver MKE15Z, driver giao tiep voi cac ngoai vi trong board           |
   * |\b src\\library\\service       | Day la thu vien cac mid layer trong source                             |
   *
   * <hr>
   *
   * Revision History
   * ------------
   * Please get in touch with \ref minh.hn0995@gmail.com or \ref github.com/minhngocha/
   *
   * Copyright Notice
   * ------------
   *
   * Copyright (C) 2023-2028 Arm Limited. All rights reserved.
   */