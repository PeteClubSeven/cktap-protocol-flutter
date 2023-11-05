import 'package:cktap_protocol/tap_protocol.dart';
import 'package:cktap_protocol_example/bloc/card_events.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:nfc_manager/nfc_manager.dart';

import '../bloc/card_bloc.dart';
import '../bloc/card_states.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => HomeScreenState();
}

class HomeScreenState extends State<HomeScreen> {
  final _textStyle = const TextStyle(fontSize: 12);
  final _spacerSmall = const SizedBox(height: 10);

  @override
  void initState() {
    super.initState();

    // Start NFC service
    NfcManager.instance.startSession(
      onDiscovered: (NfcTag tag) async {
        if (CKTapCardProtocol.isCoinkiteCard(tag)) {
          final card = await CKTapCardProtocol.instance.createCKTapCard(tag);
          final bloc = BlocProvider.of<CardBloc>(context);
          bloc.add(CardDetected(card));
        }
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Coinkite Tap Protocol Tester'),
      ),
      body: Center(
        child: BlocBuilder<CardBloc, CardState>(
          builder: (context, state) {
            if (state is DumpSatscardState) {
              return _buildDumpSatscardState(state);
            }
            if (state is DumpTapsignerState) {
              return _buildDumpTapsignerState(state);
            }
            if (state is InvalidCardState) {
              return _buildInvalidCardState(state);
            }

            assert(state is PromptCardState);
            return _buildPromptCardState();
          },
        ),
      ),
    );
  }

  Widget _buildDumpSatscardState(DumpSatscardState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            const Text(
              'CKTapCard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'internalHandle: ${state.satscard.handle.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'internalType: ${state.satscard.type.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'ident: ${state.satscard.ident}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'appletVersion: ${state.satscard.appletVersion}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'birthHeight: ${state.satscard.birthHeight.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTestnet: ${state.satscard.isTestnet.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'authDelay: ${state.satscard.authDelay.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTampered: ${state.satscard.isTampered.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isCertsChecked: ${state.satscard.isCertsChecked.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'needSetup: ${state.satscard.needSetup.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTapsigner: ${state.satscard.isTapsigner.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            const Text(
              'Satscard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'activeSlot',
              style: _textStyle,
              textAlign: TextAlign.center,
            ),
            Text(
              'index: ${state.satscard.activeSlot.index.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'status: ${state.satscard.activeSlot.status.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'address: ${state.satscard.activeSlot.address}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'privkey: ${state.satscard.activeSlot.privkey}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'pubkey: ${state.satscard.activeSlot.pubkey}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'masterPK: ${state.satscard.activeSlot.masterPK}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'chainCode: ${state.satscard.activeSlot.chainCode}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isValid: ${state.satscard.activeSlot.isValid.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'activeSlotIndex: ${state.satscard.activeSlotIndex.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'numSlots: ${state.satscard.numSlots.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'hasUnusedSlots: ${state.satscard.hasUnusedSlots.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isUsedUp: ${state.satscard.isUsedUp.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildDumpTapsignerState(DumpTapsignerState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            const Text(
              'CKTapCard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'internalHandle: ${state.tapsigner.handle.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'internalType: ${state.tapsigner.type.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'ident: ${state.tapsigner.ident}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'appletVersion: ${state.tapsigner.appletVersion}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'birthHeight: ${state.tapsigner.birthHeight.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTestnet: ${state.tapsigner.isTestnet.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'authDelay: ${state.tapsigner.authDelay.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTampered: ${state.tapsigner.isTampered.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isCertsChecked: ${state.tapsigner.isCertsChecked.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'needSetup: ${state.tapsigner.needSetup.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTapsigner: ${state.tapsigner.isTapsigner.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            const Text(
              'Tapsigner',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'numberOfBackups: ${state.tapsigner.numberOfBackups}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'derivationPath: ${state.tapsigner.derivationPath}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInvalidCardState(InvalidCardState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            Text(
              'An invalid or unsupported Coinkite NFC card has been tapped',
              style: _textStyle,
              textAlign: TextAlign.center,
            )
          ],
        ),
      ),
    );
  }

  Widget _buildPromptCardState() {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            Text(
              'Please tap your phone with a Satscard or a Tapsigner',
              style: _textStyle,
              textAlign: TextAlign.center,
            )
          ],
        ),
      ),
    );
  }
}