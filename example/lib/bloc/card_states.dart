import 'package:cktap_protocol/cktapcard.dart';

abstract class CardState {}

class DumpSatscardState extends CardState {
  final Satscard satscard;
  final Slot activeSlot;

  DumpSatscardState(this.satscard, this.activeSlot);
}

class DumpTapsignerState extends CardState {
  final Tapsigner tapsigner;

  DumpTapsignerState(this.tapsigner);
}

class InvalidCardState extends CardState {}

class PromptCardState extends CardState {}
