
#include "plansys2_pddl_parser/Instance.h"

namespace parser { namespace pddl {

void FunctionExpression::PDDLPrint( std::ostream & s, unsigned indent, const TokenStruct< std::string > & ts, const Domain & d ) const {
	ParamCond * c = d.funcs[d.funcs.index( fun->name )];

	s << "( " << fun->name;
	IntVec v( c->params.size() );
	for ( unsigned i = 0; i < v.size(); ++i ) {
		if ( ts.size() && fun->params[i] >= 0 && (unsigned)fun->params[i] < ts.size() ) s << " " << ts[fun->params[i]];
		else if (fun->params[i] >= 0 && (unsigned)fun->params[i] >= ts.size()) s << " ?" << fun->params[i];
		else s << " " << d.types[c->params[i]]->object( fun->params[i] ).first;
	}
	s << " )";
}

std::shared_ptr<tree::TreeNode> FunctionExpression::PDDLTree( const Domain & d ) const {
    std::shared_ptr<tree::FunctionNode> tree = std::make_shared<tree::FunctionNode>();
    tree->function_.name = fun->name;
    for ( unsigned i = 0; i < fun->params.size(); ++i ) {
        tree::Param param;
        param.name = "?" + std::to_string(fun->params[i]);
        param.type = d.types[fun->params[i]]->name;
        tree->function_.parameters.push_back(param);
    }
    return tree;
}

double FunctionExpression::evaluate( Instance & ins, const StringVec & par ) {
	ParamCond * c = ins.d.funcs[ins.d.funcs.index( fun->name )];

	IntVec v( c->params.size() );
	for ( unsigned i = 0; i < v.size(); ++i ) {
		std::pair< bool, unsigned > p = ins.d.types[c->params[i]]->parseObject( par[fun->params[i]] );
		if ( p.first ) v[i] = p.second;
		else {
			std::pair< bool, int > q = ins.d.types[c->params[i]]->parseConstant( par[fun->params[i]] );
			if ( q.first ) v[i] = q.second;
			else return 1;
		}
	}

	for ( unsigned i = 0; i < ins.init.size(); ++i )
		if ( ins.init[i]->name == c->name && ins.init[i]->params == v )
			return ((GroundFunc<double> *)ins.init[i])->value;
	return 1;
}

Expression * createExpression( Stringreader & f, TokenStruct< std::string > & ts, Domain & d ) {
	f.next();

	if ( f.getChar() == '(' ) {
		++f.c;
		f.next();
		std::string s = f.getToken();
		if ( s == "+" || s == "-" || s == "*" || s == "/" ) {
			CompositeExpression * ce = new CompositeExpression( s );
			ce->parse( f, ts, d );
			return ce;
		}
		else {
			f.c -= s.size();
			Function * fun = d.funcs.get( f.getToken( d.funcs ) );
			ParamCond * c = new Lifted( fun );
			for ( unsigned i = 0; i < fun->params.size(); ++i ) {
				f.next();
				c->params[i] = ts.index( f.getToken( ts ) );
			}
			f.next();
			f.assert_token( ")" );
			return new FunctionExpression( c );
		}
	}
	else if ( f.getChar() == '?' ) {  // just support duration if starts with ?
		f.assert_token( "?duration" );
		return new DurationExpression();
	}
	else {
		double d;
		std::string s = f.getToken();
		std::istringstream is( s );
		is >> d;
		return new ValueExpression( d );
	}
}

} }
