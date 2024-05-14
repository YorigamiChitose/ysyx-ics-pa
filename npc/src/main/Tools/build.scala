package main.Tools

import main.Example.adder

object build extends App {
  val firtoolLoweringOptions = Array(
    "--lowering-options=" + List(
      "disallowLocalVariables",
      "disallowPackedArrays",
      "locationInfoStyle=none"
    ).reduce(_ + "," + _)
  )
  val firtoolNormalOptions = Array("--disable-all-randomization")

  val firtoolOptions = firtoolNormalOptions ++ firtoolLoweringOptions

  println("firtool options:")
  firtoolOptions.foreach(opt => println("\t" + opt))

  circt.stage.ChiselStage.emitSystemVerilogFile(new adder(64), args, firtoolOptions)
}
