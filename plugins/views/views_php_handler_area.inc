<?php
// $Id$

/**
 * A handler to provide an area that is constructed by the administrator using PHP.
 *
 * @ingroup views_area_handlers
 */
class views_php_handler_area extends views_handler_area {

  /**
   * Only let users with use PHP permissions set/modify this handler.
   */
  function php_check_access(&$element) {
    if (!user_access('use PHP for settings')) {
      $element['#disabled'] = TRUE;
      $element['#value'] = $element['#default_value'];
      $element['#description'] .= ' <strong>' . t('Note: you do not have permission to modify this.') . '</strong>';
    }
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();

    $options['phpcode'] = array('default' => '');
    return $options;
  }

  /**
   * Implements views_handler#option_definition().
   */
  function options_form(&$form, &$form_state) {
    parent::options_form($form, $form_state);

    $form['phpcode'] = array(
      '#type' => 'textarea',
      '#title' => t('PHP code'),
      '#default_value' => $this->options['phpcode'],
      '#rows' => 5,
      '#description' =>
        t('The text that should be displayed.')
        .' '.t('Include &lt;?php ?&gt; delimiters when using PHP code.')
        .' '.t('Available variables:').'<br/>'
        .t('$data: contains the retrieved record from the database (e.g. $data->nid).').'<br/>'
        .t('$static: can be used to store reusable data per row.')
      ,
    );
    php_check_access($form['phpcode']);
  }
}
