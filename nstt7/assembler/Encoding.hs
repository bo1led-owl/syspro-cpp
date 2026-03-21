{-# LANGUAGE BinaryLiterals #-}
{-# LANGUAGE ImportQualifiedPost #-}

module Encoding (encodeProgram) where

import Control.Monad
import Control.Monad.Writer
import Data.Binary
import Data.Binary.Put
import Data.Bits
import Data.Either.Extra
import Data.Foldable
import Data.Function
import Data.Map qualified as M
import Program

encodeProgram :: Program -> Either String Put
encodeProgram (Program instructions labels) =
  traverse_ (putWord16host . runEncoder)
    <$> traverse (uncurry (encodeInstr labels)) ([0 ..] `zip` instructions)

newtype EncodedInstr = EncodedInstr {unInstr :: Word16}

instance Semigroup EncodedInstr where
  a <> b = EncodedInstr $ ((.|.) `on` unInstr) a b

instance Monoid EncodedInstr where
  mempty = EncodedInstr 0

type Encoder = Writer EncodedInstr ()

runEncoder :: Encoder -> Word16
runEncoder = unInstr . execWriter

shifted :: Word16 -> Int -> Int -> Encoder
shifted w off size =
  tell $ EncodedInstr (shiftL (w .&. shiftR (complement 0) (16 - size)) (16 - off - size))

laidOut :: [Int] -> [Word16] -> Encoder
laidOut layout parts =
  foldM_
    ( \offset (size, part) -> do
        shifted part offset size
        pure (offset + size)
    )
    0
    (layout `zip` parts)

encodeInstr :: LabelMap -> Word16 -> Instr -> Either String Encoder
encodeInstr _ _ Halt = pure $ pure ()
encodeInstr _ _ (MovReg dest src) = pure $ laidOut [7, 3, 3] [0b0000001, encodeReg dest, encodeReg src]
encodeInstr _ _ (MovImm dest src) = pure $ laidOut [1, 3, 12] [1, encodeReg dest, src]
encodeInstr _ _ (ShlReg lhs rhs) = pure $ laidOut [8, 3, 3] [0b00000100, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (ShlImm lhs rhs) = pure $ laidOut [8, 3, 4] [0b00000101, encodeReg lhs, rhs]
encodeInstr _ _ (ShrReg lhs rhs) = pure $ laidOut [8, 3, 3] [0b00000110, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (ShrImm lhs rhs) = pure $ laidOut [8, 3, 4] [0b00000111, encodeReg lhs, rhs]
encodeInstr _ _ (AddReg lhs rhs) = pure $ laidOut [7, 3, 3] [0b0000100, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (SubReg lhs rhs) = pure $ laidOut [7, 3, 3] [0b0000101, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (MulReg lhs rhs) = pure $ laidOut [7, 3, 3] [0b0000110, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (DivReg lhs rhs) = pure $ laidOut [7, 3, 3] [0b0000111, encodeReg lhs, encodeReg rhs]
encodeInstr _ _ (AddImm lhs rhs) = pure $ laidOut [5, 3, 8] [0b00100, encodeReg lhs, rhs]
encodeInstr _ _ (SubImm lhs rhs) = pure $ laidOut [5, 3, 8] [0b00101, encodeReg lhs, rhs]
encodeInstr _ _ (MulImm lhs rhs) = pure $ laidOut [5, 3, 8] [0b00110, encodeReg lhs, rhs]
encodeInstr _ _ (DivImm lhs rhs) = pure $ laidOut [5, 3, 8] [0b00111, encodeReg lhs, rhs]
encodeInstr _ _ (Load dest src offset) =
  pure $ laidOut [5, 3, 3, 3] [0b00010, encodeReg dest, encodeReg src, encodeReg offset]
encodeInstr _ _ (Store dest offset src) =
  pure $ laidOut [5, 3, 3, 3] [0b00011, encodeReg dest, encodeReg src, encodeReg offset]
encodeInstr labels i (Jmp label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 11] [0b01111, offset]
encodeInstr labels i (Blt reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01000, encodeReg reg, offset]
encodeInstr labels i (Ble reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01001, encodeReg reg, offset]
encodeInstr labels i (Bgt reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01010, encodeReg reg, offset]
encodeInstr labels i (Bge reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01011, encodeReg reg, offset]
encodeInstr labels i (Beq reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01100, encodeReg reg, offset]
encodeInstr labels i (Bne reg label) = do
  offset <- getOffset labels i label
  pure $ laidOut [5, 3, 8] [0b01101, encodeReg reg, offset]

getOffset :: LabelMap -> Word16 -> Label -> Either String Word16
getOffset labels cur label =
  (\o -> o - fromIntegral cur) <$> maybeToEither ("unknown label `" ++ label ++ "`") (M.lookup label labels)

encodeReg :: Reg -> Word16
encodeReg RZ = 0
encodeReg R1 = 1
encodeReg R2 = 2
encodeReg R3 = 3
encodeReg R4 = 4
encodeReg R5 = 5
encodeReg R6 = 6
encodeReg R7 = 7
