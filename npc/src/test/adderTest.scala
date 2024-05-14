package test.Example

import main.Example.adder

import chisel3._
import chisel3.simulator.EphemeralSimulator._
import org.scalatest.freespec.AnyFreeSpec
import org.scalatest.matchers.must.Matchers

class adderTest_8 extends AnyFreeSpec with Matchers {
  "Adder test(8)" in {
    simulate(new adder(8)) { dut =>
      for (i <- 0 until 100) {
        val a = scala.util.Random.nextInt(0xFF - 1)
        val b = scala.util.Random.nextInt(0xFF - 1)
        val c = (a + b) & 0xFF
        dut.io.in_a.poke(a.U)
        dut.io.in_b.poke(b.U)
        dut.io.out.expect(c.U)
      }
    }
  }
}

class adderTest_16 extends AnyFreeSpec with Matchers {
  "Adder test(16)" in {
    simulate(new adder(16)) { dut =>
      for (i <- 0 until 100) {
        val a = scala.util.Random.nextInt(0xFFFF - 1)
        val b = scala.util.Random.nextInt(0xFFFF - 1)
        val c = (a + b) & 0xFFFF
        dut.io.in_a.poke(a.U)
        dut.io.in_b.poke(b.U)
        dut.io.out.expect(c.U)
      }
    }
  }
}