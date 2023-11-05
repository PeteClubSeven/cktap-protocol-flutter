import 'package:cktap_protocol_example/bloc/card_bloc.dart';
import 'package:cktap_protocol_example/ui/home_screen.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends MaterialApp {
  MyApp({super.key})
      : super(
          home: BlocProvider(
            create: (_) => CardBloc(),
            child: const HomeScreen(),
          ),
        );
}

class _MyAppState extends State<MyApp> {
  @override
  void initState() {
    super.initState();

  }

  @override
  Widget build(BuildContext context) {
    return const MaterialApp(home: HomeScreen());
  }
}
