#ifndef _GPIO_
#define _GPIO_

// GPIO definitions for neon using STM8S103F3

// no option bytes

#define _msk0 0x01
#define _msk1 0x02
#define _msk2 0x04
#define _msk3 0x08
#define _msk4 0x10
#define _msk5 0x20
#define _msk6 0x40
#define _msk7 0x80

/////////////  lgtsens sensor d5 ///////////////
#define  lgtsens_ptr GPIOD
#define  lgtsens_mask _msk5

#define lgtsens_d           lgtsens_ptr->DDR
#define lgtsens_1           lgtsens_ptr->CR1
#define lgtsens_2           lgtsens_ptr->CR2
#define lgtsens_o           lgtsens_ptr->ODR
#define lgtsens_r           &= ~lgtsens_mask
#define lgtsens_s           |=  lgtsens_mask
#define lgtsens_in          lgtsens_d lgtsens_r; lgtsens_1 lgtsens_r; lgtsens_2 lgtsens_r
#define lgtsens_in_int      lgtsens_d lgtsens_r; lgtsens_1 lgtsens_r; lgtsens_2 lgtsens_s
#define lgtsens_in_pu       lgtsens_d lgtsens_r; lgtsens_1 lgtsens_s; lgtsens_2 lgtsens_r
#define lgtsens_in_pu_int   lgtsens_d lgtsens_r; lgtsens_1 lgtsens_s; lgtsens_2 lgtsens_s
#define lgtsens_out_od      lgtsens_d lgtsens_s; lgtsens_1 lgtsens_r; lgtsens_2 lgtsens_r
#define lgtsens_out_od_fast lgtsens_d lgtsens_s; lgtsens_1 lgtsens_r; lgtsens_2 lgtsens_s
#define lgtsens_out         lgtsens_d lgtsens_s; lgtsens_1 lgtsens_s; lgtsens_2 lgtsens_r
#define lgtsens_out_fast    lgtsens_d lgtsens_s; lgtsens_1 lgtsens_s; lgtsens_2 lgtsens_s
#define lgtsens_set         lgtsens_o lgtsens_s
#define lgtsens_clr         lgtsens_o lgtsens_r
#define lgtsens_toggle      lgtsens_o ^= lgtsens_mask
#define lgtsens_bit        (lgtsens_o &  lgtsens_mask)
#define lgtsens_lvl        (lgtsens_ptr->IDR & lgtsens_mask)
#define lgtsens_setto(_x)   if(_x) lgtsens_set; else lgtsens_clr

/////////////  pwm  a3 ///////////////
#define  pwm_ptr GPIOA
#define  pwm_mask _msk3

#define pwm_d           pwm_ptr->DDR
#define pwm_1           pwm_ptr->CR1
#define pwm_2           pwm_ptr->CR2
#define pwm_o           pwm_ptr->ODR
#define pwm_r           &= ~pwm_mask
#define pwm_s           |=  pwm_mask
#define pwm_in          pwm_d pwm_r; pwm_1 pwm_r; pwm_2 pwm_r
#define pwm_in_int      pwm_d pwm_r; pwm_1 pwm_r; pwm_2 pwm_s
#define pwm_in_pu       pwm_d pwm_r; pwm_1 pwm_s; pwm_2 pwm_r
#define pwm_in_pu_int   pwm_d pwm_r; pwm_1 pwm_s; pwm_2 pwm_s
#define pwm_out_od      pwm_d pwm_s; pwm_1 pwm_r; pwm_2 pwm_r
#define pwm_out_od_fast pwm_d pwm_s; pwm_1 pwm_r; pwm_2 pwm_s
#define pwm_out         pwm_d pwm_s; pwm_1 pwm_s; pwm_2 pwm_r
#define pwm_out_fast    pwm_d pwm_s; pwm_1 pwm_s; pwm_2 pwm_s
#define pwm_set         pwm_o pwm_s
#define pwm_clr         pwm_o pwm_r
#define pwm_toggle      pwm_o ^= pwm_mask
#define pwm_bit        (pwm_o &  pwm_mask)
#define pwm_lvl        (pwm_ptr->IDR & pwm_mask)
#define pwm_setto(_x)   if(_x) pwm_set; else pwm_clr

/////////////  cursens  d3 ///////////////
#define  cursens_ptr GPIOD
#define  cursens_mask _msk3

#define cursens_d           cursens_ptr->DDR
#define cursens_1           cursens_ptr->CR1
#define cursens_2           cursens_ptr->CR2
#define cursens_o           cursens_ptr->ODR
#define cursens_r           &= ~cursens_mask
#define cursens_s           |=  cursens_mask
#define cursens_in          cursens_d cursens_r; cursens_1 cursens_r; cursens_2 cursens_r
#define cursens_in_int      cursens_d cursens_r; cursens_1 cursens_r; cursens_2 cursens_s
#define cursens_in_pu       cursens_d cursens_r; cursens_1 cursens_s; cursens_2 cursens_r
#define cursens_in_pu_int   cursens_d cursens_r; cursens_1 cursens_s; cursens_2 cursens_s
#define cursens_out_od      cursens_d cursens_s; cursens_1 cursens_r; cursens_2 cursens_r
#define cursens_out_od_fast cursens_d cursens_s; cursens_1 cursens_r; cursens_2 cursens_s
#define cursens_out         cursens_d cursens_s; cursens_1 cursens_s; cursens_2 cursens_r
#define cursens_out_fast    cursens_d cursens_s; cursens_1 cursens_s; cursens_2 cursens_s
#define cursens_set         cursens_o cursens_s
#define cursens_clr         cursens_o cursens_r
#define cursens_toggle      cursens_o ^= cursens_mask
#define cursens_bit        (cursens_o &  cursens_mask)
#define cursens_lvl        (cursens_ptr->IDR & cursens_mask)
#define cursens_setto(_x)   if(_x) cursens_set; else cursens_clr

/////////////  button  d2 ///////////////
#define  button_ptr GPIOD
#define  button_mask _msk2

#define button_d           button_ptr->DDR
#define button_1           button_ptr->CR1
#define button_2           button_ptr->CR2
#define button_o           button_ptr->ODR
#define button_r           &= ~button_mask
#define button_s           |=  button_mask
#define button_in          button_d button_r; button_1 button_r; button_2 button_r
#define button_in_int      button_d button_r; button_1 button_r; button_2 button_s
#define button_in_pu       button_d button_r; button_1 button_s; button_2 button_r
#define button_in_pu_int   button_d button_r; button_1 button_s; button_2 button_s
#define button_out_od      button_d button_s; button_1 button_r; button_2 button_r
#define button_out_od_fast button_d button_s; button_1 button_r; button_2 button_s
#define button_out         button_d button_s; button_1 button_s; button_2 button_r
#define button_out_fast    button_d button_s; button_1 button_s; button_2 button_s
#define button_set         button_o button_s
#define button_clr         button_o button_r
#define button_toggle      button_o ^= button_mask
#define button_bit        (button_o &  button_mask)
#define button_lvl        (button_ptr->IDR & button_mask)
#define button_setto(_x)   if(_x) button_set; else button_clr

/////////////  enca  c7 ///////////////
#define  enca_ptr GPIOC
#define  enca_mask _msk7

#define enca_d           enca_ptr->DDR
#define enca_1           enca_ptr->CR1
#define enca_2           enca_ptr->CR2
#define enca_o           enca_ptr->ODR
#define enca_r           &= ~enca_mask
#define enca_s           |=  enca_mask
#define enca_in          enca_d enca_r; enca_1 enca_r; enca_2 enca_r
#define enca_in_int      enca_d enca_r; enca_1 enca_r; enca_2 enca_s
#define enca_in_pu       enca_d enca_r; enca_1 enca_s; enca_2 enca_r
#define enca_in_pu_int   enca_d enca_r; enca_1 enca_s; enca_2 enca_s
#define enca_out_od      enca_d enca_s; enca_1 enca_r; enca_2 enca_r
#define enca_out_od_fast enca_d enca_s; enca_1 enca_r; enca_2 enca_s
#define enca_out         enca_d enca_s; enca_1 enca_s; enca_2 enca_r
#define enca_out_fast    enca_d enca_s; enca_1 enca_s; enca_2 enca_s
#define enca_set         enca_o enca_s
#define enca_clr         enca_o enca_r
#define enca_toggle      enca_o ^= enca_mask
#define enca_bit        (enca_o &  enca_mask)
#define enca_lvl        (enca_ptr->IDR & enca_mask)
#define enca_setto(_x)   if(_x) enca_set; else enca_clr

/////////////  encb  c6 ///////////////
#define  encb_ptr GPIOC
#define  encb_mask _msk6

#define encb_d           encb_ptr->DDR
#define encb_1           encb_ptr->CR1
#define encb_2           encb_ptr->CR2
#define encb_o           encb_ptr->ODR
#define encb_r           &= ~encb_mask
#define encb_s           |=  encb_mask
#define encb_in          encb_d encb_r; encb_1 encb_r; encb_2 encb_r
#define encb_in_int      encb_d encb_r; encb_1 encb_r; encb_2 encb_s
#define encb_in_pu       encb_d encb_r; encb_1 encb_s; encb_2 encb_r
#define encb_in_pu_int   encb_d encb_r; encb_1 encb_s; encb_2 encb_s
#define encb_out_od      encb_d encb_s; encb_1 encb_r; encb_2 encb_r
#define encb_out_od_fast encb_d encb_s; encb_1 encb_r; encb_2 encb_s
#define encb_out         encb_d encb_s; encb_1 encb_s; encb_2 encb_r
#define encb_out_fast    encb_d encb_s; encb_1 encb_s; encb_2 encb_s
#define encb_set         encb_o encb_s
#define encb_clr         encb_o encb_r
#define encb_toggle      encb_o ^= encb_mask
#define encb_bit        (encb_o &  encb_mask)
#define encb_lvl        (encb_ptr->IDR & encb_mask)
#define encb_setto(_x)   if(_x) encb_set; else encb_clr

/////////////  bsens  C4 ///////////////
#define  bsens_ptr GPIOC
#define  bsens_mask _msk4

#define bsens_d           bsens_ptr->DDR
#define bsens_1           bsens_ptr->CR1
#define bsens_2           bsens_ptr->CR2
#define bsens_o           bsens_ptr->ODR
#define bsens_r           &= ~bsens_mask
#define bsens_s           |=  bsens_mask
#define bsens_in          bsens_d bsens_r; bsens_1 bsens_r; bsens_2 bsens_r
#define bsens_in_int      bsens_d bsens_r; bsens_1 bsens_r; bsens_2 bsens_s
#define bsens_in_pu       bsens_d bsens_r; bsens_1 bsens_s; bsens_2 bsens_r
#define bsens_in_pu_int   bsens_d bsens_r; bsens_1 bsens_s; bsens_2 bsens_s
#define bsens_out_od      bsens_d bsens_s; bsens_1 bsens_r; bsens_2 bsens_r
#define bsens_out_od_fast bsens_d bsens_s; bsens_1 bsens_r; bsens_2 bsens_s
#define bsens_out         bsens_d bsens_s; bsens_1 bsens_s; bsens_2 bsens_r
#define bsens_out_fast    bsens_d bsens_s; bsens_1 bsens_s; bsens_2 bsens_s
#define bsens_set         bsens_o bsens_s
#define bsens_clr         bsens_o bsens_r
#define bsens_toggle      bsens_o ^= bsens_mask
#define bsens_bit        (bsens_o &  bsens_mask)
#define bsens_lvl        (bsens_ptr->IDR & bsens_mask)
#define bsens_setto(_x)   if(_x) bsens_set; else bsens_clr

/////////////  pwron  c3 ///////////////
#define  pwron_ptr GPIOC
#define  pwron_mask _msk3

#define pwron_d           pwron_ptr->DDR
#define pwron_1           pwron_ptr->CR1
#define pwron_2           pwron_ptr->CR2
#define pwron_o           pwron_ptr->ODR
#define pwron_r           &= ~pwron_mask
#define pwron_s           |=  pwron_mask
#define pwron_in          pwron_d pwron_r; pwron_1 pwron_r; pwron_2 pwron_r
#define pwron_in_int      pwron_d pwron_r; pwron_1 pwron_r; pwron_2 pwron_s
#define pwron_in_pu       pwron_d pwron_r; pwron_1 pwron_s; pwron_2 pwron_r
#define pwron_in_pu_int   pwron_d pwron_r; pwron_1 pwron_s; pwron_2 pwron_s
#define pwron_out_od      pwron_d pwron_s; pwron_1 pwron_r; pwron_2 pwron_r
#define pwron_out_od_fast pwron_d pwron_s; pwron_1 pwron_r; pwron_2 pwron_s
#define pwron_out         pwron_d pwron_s; pwron_1 pwron_s; pwron_2 pwron_r
#define pwron_out_fast    pwron_d pwron_s; pwron_1 pwron_s; pwron_2 pwron_s
#define pwron_set         pwron_o pwron_s
#define pwron_clr         pwron_o pwron_r
#define pwron_toggle      pwron_o ^= pwron_mask
#define pwron_bit        (pwron_o &  pwron_mask)
#define pwron_lvl        (pwron_ptr->IDR & pwron_mask)
#define pwron_setto(_x)   if(_x) pwron_set; else pwron_clr

#endif
