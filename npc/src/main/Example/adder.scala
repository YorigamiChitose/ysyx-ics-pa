package main.Example

import chisel3._

class adderBundle(width: Int) extends Bundle {
    val in_a = Input(UInt(width.W))
    val in_b = Input(UInt(width.W))
    val out = Output(UInt(width.W))
}

class adder(width: Int) extends Module {
    val io = IO(new adderBundle(width))

    io.out := io.in_a + io.in_b
}