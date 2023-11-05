import 'package:cktap_protocol/cktapcard.dart';

abstract class CardEvent {}

class CardDetected extends CardEvent {
  final CKTapCard card;

  CardDetected(this.card);
}
